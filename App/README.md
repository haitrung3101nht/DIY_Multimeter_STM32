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

Dùng `TFT_SetText(id, y, text, color)` để cập nhật một trong 6 ô cố định.
Mỗi ô chứa tối đa 13 ký tự ASCII, font 16x28 có làm mịn cạnh, nền đen.
Giữ nguyên tọa độ y cho mỗi id, không cho các ô chồng nhau.
Ứng dụng có bốn trang, chọn bằng 0/4/8/A. Xem UI/README.md để sửa
và thêm chức năng. Những phím khác không có tác dụng điều hướng.

`TFT_Process()` chỉ gửi một dòng 208 pixel mỗi lần. Nội dung không đổi
không gửi lại. Cập nhật đến khi đang vẽ được giữ cho lượt tiếp theo.
Gửi màu nền và màu chữ cùng lúc, không xóa đen cả ô trước khi vẽ.
Các hàm TFT_DrawString/FillRect cũ vẫn là hàm đồng bộ, không dùng trong
đường xử lý định kỳ của app mới.

Font sinh sẵn trong flash; build firmware không cần Pillow. Muốn sinh lại:
`python3 scripts/generate_tft_font.py` (cần Pillow và DejaVu Sans Mono).
Giấy phép font ở Modules/TFT/FONT_LICENSE.txt.

## Giới hạn và kiểm tra

Độ phân giải vật lý vẫn là 240x240. Tăng tốc SPI không đồng nghĩa tăng
refresh nội bộ của panel. Một dòng 416 byte mất tối thiểu 0.416 ms trên
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
