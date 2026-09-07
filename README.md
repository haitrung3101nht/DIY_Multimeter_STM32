# STMproject — Ngày 1: cấu trúc project và GPIO

Bài học STM32F411 dùng HAL, module `led` đơn giản và hai hàm
`app_init()` / `app_process()`. Mặc định LED toggle mỗi 500 ms.

## Luồng chương trình

```text
main()
  HAL_Init()
  SystemClock_Config()
  MX_GPIO_Init()
  MX_I2C1_Init()          cấu hình cũ được giữ lại
  app_init()             gọi một lần
  while (1)              super loop: lặp mãi
    app_process()

Application → led_on/off/toggle() → HAL_GPIO_xxx() → GPIO peripheral → pin
```

`HAL` là thư viện điều khiển phần cứng của ST. GPIO output điều khiển mức
điện áp chân; GPIO input đọc mức logic tại chân. Clock hệ thống HSI 16 MHz
và clock GPIO vẫn được giữ vì cần cho MCU/peripheral hoạt động.
Phần ứng dụng đồng hồ (`clock_time`, `clock_view`, test đồng hồ) đã xóa.
Driver OLED, I2C, CMSIS, HAL và các thư mục mở rộng vẫn giữ nguyên;
app ngày 1 không gọi OLED.

## Các file cần học

| File | Vai trò |
|---|---|
| `Core/Src/main.c` | Khởi tạo hệ thống và chạy super loop |
| `Core/Src/app.c` | Logic bài học, chọn chế độ thực hành |
| `Core/Src/led.c`, `Core/Inc/led.h` | Module LED: init, on, off, toggle |
| `Core/Src/gpio.c` | Cấu hình GPIO output/input |
| `Core/Inc/main.h` | Tên chân LED và button |
| `STMproject.ioc` | Cấu hình CubeMX, đã đồng bộ LED và button |

`led_init()` đặt LED về tắt, gọi sau `MX_GPIO_Init()`.
Chưa cần interface, callback hay module button riêng.

## Phần cứng

Giữ giả định Black Pill của project cũ: LED PC13 **active low**:
`GPIO_PIN_RESET` làm sáng, `GPIO_PIN_SET` làm tắt. Nếu board của bạn khác,
đối chiếu chân LED và cực tính trước khi thử.

Button bài học là **nút ngoài nối PA0 với GND**. PA0 được cấu hình input
pull-up: thả đọc SET (1), nhấn đọc RESET (0). Không cần điện trở kéo ngoài.
Không mặc định dùng nút BOOT/RESET trên board; bài này chỉ dùng nút ngoài PA0.

## Ba bài thực hành

Đổi dòng `#define APP_LESSON LESSON_TOGGLE` trong `Core/Src/app.c`,
build và nạp lại sau mỗi lần đổi:

| Giá trị | Kết quả mong đợi |
|---|---|
| `LESSON_ON_OFF` | Gọi `led_on()`, chờ 500 ms, `led_off()`, chờ 500 ms |
| `LESSON_TOGGLE` | Gọi `led_toggle()` mỗi 500 ms; một chu kỳ sáng/tắt dài 1 giây |
| `LESSON_BUTTON` | Giữ nút thì sáng, thả nút thì tắt |

Để thử LED sáng/tắt cố định, thay nội dung nhánh ON/OFF bằng duy nhất
`led_on();` hoặc `led_off();`. Logic thực hành nằm trong `app.c`.

`HAL_Delay()` chặn xử lý trong thời gian chờ, đủ cho bài đầu tiên.
Button được đọc liên tục (polling), chưa chống dội; chưa dùng để đếm lần nhấn.

## Build và nạp

```sh
cmake --preset Debug
cmake --build --preset Debug
```

Firmware: `build/Debug/STMproject.elf`. Nạp qua ST-Link theo
`howtoruncode.txt` hoặc chạy:

```sh
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/Debug/STMproject.elf verify reset exit"
```

Sau khi nạp, kiểm tra lần lượt ba chế độ ở bảng trên. Build thành công
chưa xác nhận được chân LED/nút và hoạt động thực tế trên board.

Các lời gọi app nằm trong vùng USER CODE để CubeMX giữ khi sinh lại code.
Đổi chân trong CubeMX thì giữ nhãn `LED`, `BUTTON` và cập nhật cực tính
trong module nếu phần cứng thay đổi.
