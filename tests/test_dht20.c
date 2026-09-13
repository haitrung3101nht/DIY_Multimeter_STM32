#include "dht20.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum EventKind { WRITE, READ, DELAY };
typedef struct {
    enum EventKind kind;
    uint8_t bytes[7];
    uint16_t size;
    uint32_t milliseconds;
    bool success;
} Event;
static Event events[64];
static unsigned event_count, event_index;

static void expect_bytes(enum EventKind kind, const uint8_t *bytes, uint16_t size)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    Event *event = &events[event_count++];
    *event = (Event){.kind = kind, .size = size, .success = true};
    assert(size <= sizeof(event->bytes));
    memcpy(event->bytes, bytes, size);
}

static void expect_delay(uint32_t milliseconds)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count++] = (Event){.kind = DELAY, .milliseconds = milliseconds};
}

static Event *next_event(enum EventKind kind)
{
    assert(event_index < event_count);
    Event *event = &events[event_index++];
    assert(event->kind == kind);
    return event;
}

static bool write_mock(const uint8_t *data, uint16_t size)
{
    Event *event = next_event(WRITE);
    assert(data && size == event->size);
    assert(memcmp(data, event->bytes, size) == 0);
    return event->success;
}

static bool read_mock(uint8_t *data, uint16_t size)
{
    Event *event = next_event(READ);
    assert(data && size == event->size);
    if (event->success) memcpy(data, event->bytes, size);
    return event->success;
}

static void delay_mock(uint32_t milliseconds)
{
    assert(next_event(DELAY)->milliseconds == milliseconds);
}

static void clear_script(void)
{
    assert(event_index == event_count);
    event_count = event_index = 0;
}

static void expect_status(uint8_t status)
{
    /* Datasheet bus frame: START, 0x71 (address + read), status, NACK, STOP.
     * The read callback supplies the address phase; any write is a test failure. */
    expect_bytes(READ, &status, 1);
}

static void expect_init(uint8_t status, uint8_t final_status)
{
    expect_delay(100);
    expect_status(status);
    if (status & 0x80U) return;
    if ((status & 0x18U) != 0x18U) {
        const uint8_t registers[] = {0x1B, 0x1C, 0x1E};
        for (unsigned i = 0; i < sizeof(registers); ++i) {
            const uint8_t command[] = {registers[i], 0x00, 0x00};
            const uint8_t reply[] = {0x08, (uint8_t)(0x40U + i), (uint8_t)(0x12U + i)};
            const uint8_t restore[] = {(uint8_t)(0xB0U | registers[i]), reply[1], reply[2]};
            expect_bytes(WRITE, command, sizeof(command));
            expect_delay(5);
            expect_bytes(READ, reply, sizeof(reply));
            expect_delay(10);
            expect_bytes(WRITE, restore, sizeof(restore));
            expect_delay(5);
        }
        expect_delay(10);
        expect_status(final_status);
        if ((final_status & 0x98U) != 0x18U) return;
    }
    expect_delay(10);
}

static void expect_start(void)
{
    const uint8_t command[] = {0xAC, 0x33, 0x00};
    expect_bytes(WRITE, command, sizeof(command));
}

static void assert_disabled(void)
{
    DHT20_Sample sample = {.temperature_tenths = -99, .humidity_tenths = 42};
    assert(!DHT20_Start());
    assert(DHT20_ReadSample(&sample) == DHT20_ERROR);
    assert(sample.temperature_tenths == -99 && sample.humidity_tenths == 42);
}

static void test_initialization(void)
{
    assert_disabled();
    assert(!DHT20_Init(NULL, read_mock, delay_mock));
    assert_disabled();
    assert(!DHT20_Init(write_mock, NULL, delay_mock));
    assert_disabled();
    assert(!DHT20_Init(write_mock, read_mock, NULL));
    assert_disabled();

    expect_init(0x18, 0);
    assert(DHT20_Init(write_mock, read_mock, delay_mock));
    clear_script();
    expect_init(0x08, 0x18); /* Calibration bit alone is insufficient at init. */
    assert(DHT20_Init(write_mock, read_mock, delay_mock));
    clear_script();

    /* Every failed transfer, including recovery/recheck, must abort initialization. */
    expect_init(0x00, 0x18);
    unsigned full_count = event_count;
    event_count = 0;
    for (unsigned failure = 0; failure < full_count; ++failure) {
        expect_init(0x00, 0x18);
        if (events[failure].kind == DELAY) {
            event_count = 0;
            continue;
        }
        events[failure].success = false;
        event_count = failure + 1;
        assert(!DHT20_Init(write_mock, read_mock, delay_mock));
        assert_disabled();
        clear_script();
    }

    const uint8_t bad_statuses[] = {0x00, 0x08, 0x10, 0x98, 0xFF};
    for (unsigned i = 0; i < sizeof(bad_statuses); ++i) {
        expect_init(0x00, bad_statuses[i]);
        assert(!DHT20_Init(write_mock, read_mock, delay_mock));
        assert_disabled();
        clear_script();
    }
    expect_init(0x98, 0x18); /* Never reset registers during an active conversion. */
    assert(!DHT20_Init(write_mock, read_mock, delay_mock));
    assert_disabled();
    clear_script();
}

static const uint8_t normal_packet[] = {0x18, 0x80, 0x00, 0x06, 0x00, 0x00, 0x23};

static void test_measurements(void)
{
    expect_init(0x18, 0);
    assert(DHT20_Init(write_mock, read_mock, delay_mock));
    clear_script();
    DHT20_Sample sample = {.temperature_tenths = 123, .humidity_tenths = 456};
    assert(DHT20_ReadSample(&sample) == DHT20_ERROR); /* No request yet. */
    assert(sample.temperature_tenths == 123 && sample.humidity_tenths == 456);

    const struct {
        uint8_t packet[7];
        int16_t temperature;
        uint16_t humidity;
    } vectors[] = {
        /* Hard-coded CRCs from polynomial division, independent of driver CRC code. */
        {{0x18, 0x80, 0x00, 0x06, 0x00, 0x00, 0x23}, 250, 500},
        {{0x18, 0x80, 0x00, 0x02, 0x00, 0x00, 0x0A}, -250, 500},
        {{0x18, 0xAB, 0xCD, 0xE1, 0x35, 0x79, 0x31}, -349, 671},
        {{0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35}, -500, 0},
        {{0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x17}, 1500, 1000},
        {{0x08, 0x80, 0x00, 0x06, 0x00, 0x00, 0xA6}, 250, 500}
    };
    for (unsigned i = 0; i < sizeof(vectors) / sizeof(vectors[0]); ++i) {
        expect_start();
        assert(DHT20_Start()); /* No conversion delay inside Start. */
        assert(DHT20_ReadSample(NULL) == DHT20_ERROR);
        expect_bytes(READ, vectors[i].packet, 7);
        assert(DHT20_ReadSample(&sample) == DHT20_OK);
        assert(sample.temperature_tenths == vectors[i].temperature);
        assert(sample.humidity_tenths == vectors[i].humidity);
        assert(DHT20_ReadSample(&sample) == DHT20_ERROR); /* Cannot reuse old data. */
        clear_script();
    }

    expect_start();
    assert(DHT20_Start());
    const uint8_t busy_packet[] = {0x98, 0, 0, 0, 0, 0, 0};
    for (unsigned i = 0; i < 2; ++i) {
        expect_bytes(READ, busy_packet, 7);
        assert(DHT20_ReadSample(&sample) == DHT20_BUSY);
        assert(sample.temperature_tenths == 250 && sample.humidity_tenths == 500);
    }
    expect_bytes(READ, normal_packet, sizeof(normal_packet));
    assert(DHT20_ReadSample(&sample) == DHT20_OK);
    clear_script();
}

static void test_measurement_errors(void)
{
    DHT20_Sample sample = {.temperature_tenths = -99, .humidity_tenths = 42};
    const uint8_t uncalibrated_packet[] = {0x10, 0x80, 0, 0x06, 0, 0, 0xF9};
    for (unsigned fault = 0; fault < 4; ++fault) {
        expect_start();
        if (fault == 0) events[event_count - 1].success = false;
        assert(DHT20_Start() == (fault != 0));
        if (fault != 0) {
            expect_bytes(READ, fault == 3 ? uncalibrated_packet : normal_packet, 7);
            if (fault == 1) events[event_count - 1].success = false;
            if (fault == 2) events[event_count - 1].bytes[2] ^= 0x01; /* Corrupted payload. */
        }
        assert(DHT20_ReadSample(&sample) == DHT20_ERROR);
        assert(sample.temperature_tenths == -99 && sample.humidity_tenths == 42);
        assert(DHT20_ReadSample(&sample) == DHT20_ERROR);
        clear_script();
    }

    /* After a busy timeout the application may issue a fresh request. */
    expect_start();
    assert(DHT20_Start());
    expect_start();
    assert(DHT20_Start());
    expect_bytes(READ, normal_packet, sizeof(normal_packet));
    assert(DHT20_ReadSample(&sample) == DHT20_OK);
    clear_script();
    assert(!DHT20_Init(NULL, read_mock, delay_mock)); /* Failed reinit invalidates old state. */
    assert_disabled();
}

int main(void)
{
    test_initialization();
    test_measurements();
    test_measurement_errors();
    assert(event_index == event_count);
    puts("DHT20 tests passed");
    return 0;
}
