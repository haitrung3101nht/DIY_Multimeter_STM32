# TFT, keypad và DHT20

`App_Init()` khởi tạo một lần; `App_Process()` chạy mỗi vòng main.

- Keypad: quét theo mốc 2 ms, debounce 20 ms. Giữ nguyên keymap người dùng.
- LED: đảo trạng thái theo mốc 500 ms.
- DHT20: kiểm tra sẵn sàng, chờ 10 ms bằng trạng thái, gửi lệnh, trở lại
  vòng lặp; đọc sau 85 ms, kiểm tra CRC. Busy tối đa 200 ms; lỗi thử lại
  chu kỳ sau (2 giây). Không gọi HAL_Delay trong đường xử lý định kỳ.
- TFT: SPI1 8 MHz (HSI/APB2 16 MHz, prescaler 2), mode 3, xoay 90 độ.
  Cấu hình tốc độ đã đồng bộ trong file .ioc.

## Vẽ chữ mượt

Dùng `TFT_SetText(id, x, y, text, color)` hoặc `UI_Text` trong các màn hình.
Có tối đa 16 thành phần chữ, bố trí tự do theo pixel, font 16x28 làm mịn,
tối đa 31 ký tự mỗi thành phần (cắt ở cạnh màn hình). Cùng ID cập nhật hoặc
di chuyển thành phần; vùng cũ được dựng lại. Xem UI/README.md.

`TFT_Process()` dựng lại và gửi một dòng 240 pixel bị thay đổi mỗi lần.
Không gửi lại nội dung không đổi; xử lý được nhiều chữ trên cùng dòng,
di chuyển, xóa, chồng chữ. Không dùng bộ đệm toàn màn hình.

Font sinh sẵn trong flash; build firmware không cần Pillow. Muốn sinh lại:
`python3 scripts/generate_tft_font.py` (cần Pillow và DejaVu Sans Mono).
Giấy phép font ở Modules/TFT/FONT_LICENSE.txt.

## Giới hạn và kiểm tra

Độ phân giải vật lý vẫn là 240x240. Tăng tốc SPI không đồng nghĩa tăng
refresh nội bộ của panel. Một dòng 480 byte mất tối thiểu 0.480 ms trên
bus 8 MHz; thời gian thực còn gồm render, lệnh và HAL. Đây là xử lý xen kẽ,
không phải các luồng song song hay đảm bảo thời gian thực cứng. I2C/SPI
vẫn là HAL polling có timeout; lỗi bus có thể làm trễ vài ms hoặc hơn do
các bước chờ bên trong HAL. Không dùng DMA và chưa đo FPS trên board.

Chạy:

```
python3 tests/test_app_scheduler.py
python3 tests/test_tft_fields.py
cmake --preset Debug
cmake --build --preset Debug
```

Sau khi nạp: thử các phím trong khi số đo cập nhật, kiểm tra LED, tháo/nối
lại DHT20 để kiểm tra phục hồi. Nếu TFT có nhiễu ở 8 MHz, thử prescaler 4
(4 MHz) trong CubeMX rồi generate lại; dùng dây SPI ngắn và mass tốt.
