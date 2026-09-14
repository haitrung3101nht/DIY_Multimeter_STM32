# STMproject — LED blink

Dự án nền STM32F411CEU6 dùng STM32 HAL và CMake, chỉ nhấp nháy LED PC13.
LED active-low: mức thấp sáng, mức cao tắt. Đảo trạng thái mỗi 500 ms
(sáng 500 ms, tắt 500 ms). Clock dùng HSI nội 16 MHz, giữ SWD để debug.

## Build

Cần `arm-none-eabi-gcc`, CMake và Ninja:

```sh
cmake --preset Debug
cmake --build --preset Debug
```

Firmware: `build/Debug/STMproject.elf`.
Trong VS Code, dùng task `STM32: build Debug`; nhấn F5 để nạp/debug qua
ST-Link (cần Cortex-Debug, OpenOCD và gdb-multiarch).

## Tự thêm module

- Logic nhấp nháy nằm trong `Core/Src/main.c`, vùng `USER CODE BEGIN 3`.
- Cấu hình LED nằm trong `Core/Src/gpio.c`, tên chân trong `Core/Inc/main.h`.
- Thêm ngoại vi bằng `STMproject.ioc` rồi generate code bằng STM32CubeMX.
- Thêm file `.c` vào `target_sources` và đường dẫn header vào
  `target_include_directories` trong `CMakeLists.txt`.
- Đặt code tự viết trong các vùng `USER CODE` để giữ khi generate lại.

Ví dụ ban đầu dùng `HAL_Delay(500)`, nên vòng lặp chờ trong thời gian này.
Khi thêm module cần chạy liên tục, có thể chuyển sang định thời bằng `HAL_GetTick()`.
