"""Host regression test for cooperative application scheduling (requires cc)."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as directory:
    p = Path(directory)
    (p/'main.h').write_text('''#include <stdint.h>
#define LED_GPIO_Port 0
#define LED_Pin 0
uint32_t HAL_GetTick(void);
void HAL_GPIO_TogglePin(int, int);
''')
    (p/'stm32f4xx_hal.h').write_text('#include <stdint.h>\n')
    (p/'test.c').write_text(r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "app.h"
#include "dht20.h"
static uint32_t tick, started;
static unsigned scans, toggles, starts, reads, rows, page_switches;
static int mode;
static char fields[6][32];
static char key;
uint32_t HAL_GetTick(void) { return tick; }
void HAL_GPIO_TogglePin(int a, int b) { (void)a; (void)b; toggles++; }
void Keypad_Init(void) {}
char Keypad_GetKey(void) { scans++; char k=key; key=0; return k; }
void TFT_Init(void) {}
void TFT_Process(void) { rows++; }
void TFT_InvalidateText(void) { page_switches++; }
void TFT_SetText(uint8_t id, uint16_t y, const char *s, uint16_t c) {
    (void)y; (void)c;
    assert(id<6 && strlen(s)<=13);
    snprintf(fields[id],sizeof(fields[id]),"%s",s);
}
DHT20_Status DHT20_CheckReady(void) { return mode==2 ? DHT20_ERROR_I2C : DHT20_OK; }
DHT20_Status DHT20_StartMeasurement(void) { starts++; started=tick; return DHT20_OK; }
DHT20_Status DHT20_ReadResult(DHT20_Data *d) {
    assert((uint32_t)(tick-started)>=85); reads++;
    if(mode==1) return DHT20_BUSY;
    d->temperature=-11.5f; d->humidity=62.3f; return DHT20_OK;
}
static void run(unsigned length) { for(unsigned i=0;i<length;i++){App_Process();tick++;} }
int main(void) {
    App_Init(); assert(strcmp(fields[0],"TEMPERATURE")==0);
    run(20); key='4'; run(2);
    assert(strcmp(fields[0],"ALPHABET")==0 && reads==0); /* Key received during conversion. */
    run(1000);
    assert(reads==1 && toggles==2 && scans>=500 && rows==1022);
    assert(strcmp(fields[1],"ABCDEFGHIJKLM")==0); /* Sensor cannot overwrite ABC. */
    unsigned switches=page_switches;
    key='B'; run(2); key='4'; run(2); assert(page_switches==switches);
    key='8'; run(2); assert(strcmp(fields[0],"FUNCTION 3")==0);
    key='A'; run(2); assert(strcmp(fields[0],"FUNCTION 4")==0);
    key='0'; run(2); assert(strcmp(fields[1],"-11.5 C")==0);
    mode=1; run(3200); assert(reads<40); /* Busy retries bounded; later cycle resumes. */
    unsigned old_scans=scans, old_starts=starts;
    mode=2; run(3000); assert(scans-old_scans>=1499 && starts==old_starts);
    mode=0; tick=UINT32_MAX-50; App_Init(); unsigned old_reads=reads;
    run(300); assert(reads==old_reads+1); /* Conversion crosses tick wrap. */
    puts("PASS: keypad during conversion, LED, busy timeout, disconnect, recovery, tick wrap");
}
''')
    command = ['cc','-std=c11','-Wall','-Wextra','-Werror',f'-I{p}']
    for path in ['App/Inc','App/UI/Inc','Modules/DHT20/Inc','Modules/TFT/Inc','Modules/keypad/Inc']:
        command.append(f'-I{root/path}')
    subprocess.run(command+[str(root/'App/Src/app.c')]+[str(f) for f in sorted((root/'App/UI/Src').glob('*.c'))]+[str(p/'test.c'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
