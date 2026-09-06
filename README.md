# STMproject — đồng hồ OLED trên STM32F411 Black Pill

Bản đầu hiển thị `HH:MM:SS`, bắt đầu từ `00:00:00` sau khi bật nguồn/reset,
quay về 0 sau 24 giờ. Dùng HAL_GetTick/SysTick và HSI 16 MHz hiện có;
chưa có đặt giờ, RTC hay lưu giờ khi mất nguồn. Độ chính xác phụ thuộc HSI.

## Nối dây

Giả định màn hình **SSD1306, 128×32, I2C, địa chỉ 7-bit 0x3C**.
Kích thước 0.91 inch chưa đủ để xác nhận controller; cần đối chiếu module thực tế.

| OLED | Black Pill |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | PB6 |
| SDA | PB7 |

PB6/PB7 dùng I2C1, AF4, 100 kHz. Tham khảo bảng alternate function trong
[datasheet STM32F411](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf).
Bus cần điện trở kéo lên 3V3: nếu module chưa có, thêm khoảng 4.7 kΩ
cho mỗi đường SCL và SDA. Đấu theo nhãn chân trên module.
Địa chỉ có thể đổi sang `0x3D` trong `BSP/Inc/board_oled.h`.

## Vai trò các file

| File | Trách nhiệm |
|---|---|
| `Core/Src/main.c` | Khởi tạo HAL, clock, GPIO; gọi App_Init/App_Process |
| `App/Src/app.c` | Điều phối thời gian, màn hình và thử lại khi lỗi |
| `App/Src/clock_time.c` | Đếm thời gian, xử lý tràn tick, định dạng HH:MM:SS; không phụ thuộc HAL |
| `Core/Src/i2c.c` | CubeMX cấu hình chân GPIO, I2C1 và quản lý handle hi2c1 |
| `BSP/Src/board_oled.c` | Kiểm tra OLED và truyền dữ liệu qua hi2c1 của CubeMX |
| `Display/Src/ssd1306.c` | Lệnh SSD1306, framebuffer 512 byte, cập nhật màn hình |
| `Display/Src/clock_view.c` | Font số, bố cục và vẽ giờ; không phụ thuộc HAL |
| `tests/test_clock.c` | Kiểm tra logic thời gian trên máy tính |

Mỗi module có header tương ứng trong thư mục `Inc` cùng cấp.
Driver SSD1306 nhận callback truyền dữ liệu để có thể đổi transport.
Có thể thay clock_time bằng nguồn RTC hoặc thêm màn hình đo điện riêng sau này.

## Build và chạy

```sh
cmake --preset Debug
cmake --build --preset Debug
```

Nạp `build/Debug/STMproject.elf` bằng quy trình ST-Link hiện có.
Sau khi nạp, màn hình phải hiển thị 00:00:00 và tăng mỗi giây.
Kiểm tra thực tế chuyển 00:00:59 → 00:01:00 và reset về 00:00:00.
LED PC13 sáng khi giao tiếp màn hình lỗi; chương trình thử khởi tạo lại mỗi giây.
Nếu màn hình trống, kiểm tra nguồn, dây, điện trở kéo lên, địa chỉ và controller.
ACK I2C thành công không đảm bảo module đúng loại hay hình đã hiển thị.

Kiểm tra logic trên máy tính:

```sh
cc -std=c11 -Wall -Wextra -Werror -IApp/Inc tests/test_clock.c App/Src/clock_time.c -o /tmp/stm-clock-test
/tmp/stm-clock-test
```

## CubeMX và quản lý cấu hình

I2C1 được quản lý trong CubeMX (`STMproject.ioc`): PB6=SCL, PB7=SDA,
100 kHz. `main.c` gọi `MX_I2C1_Init()` trước `App_Init()`.
BSP dùng `hi2c1` từ `Core/Inc/i2c.h`, không tự cấu hình GPIO hoặc reset I2C.
Khi thử lại màn hình, BSP kiểm tra trạng thái bus và ACK của OLED;
không thực hiện phục hồi bus bị giữ thấp bằng xung GPIO.

CMake do CubeMX sinh quản lý nguồn HAL I2C; `stm32f4xx_hal_conf.h` bật module.
CMake cấp cao chỉ thêm nguồn App/BSP/Display, tránh biên dịch HAL I2C trùng.
Các lời gọi App nằm trong USER CODE để được giữ khi sinh lại Core.
Muốn đổi chân hoặc tốc độ I2C1, chỉnh trong CubeMX rồi sinh code lại.

Build và kiểm tra trên máy tính không thay thế kiểm tra màn hình trên phần cứng.
