# Bốn chức năng và cách mở rộng

| Phím | Màn hình | File |
|---|---|---|
| 0 | Nhiệt độ, độ ẩm; mặc định khi khởi động | Src/screen_climate.c |
| 4 | Bảng chữ cái Latin A–Z và a–z | Src/screen_alphabet.c |
| 8 | Chức năng 3, chưa gán nghiệp vụ | Src/screen_function3.c |
| A | Chức năng 4, chưa gán nghiệp vụ | Src/screen_function4.c |

Chỉ bốn phím này có tác dụng điều hướng. Giữ nguyên keymap người dùng
trong Modules/keypad/Src/keypad.c: cột đầu là 0, 4, 8, A.
Phím đang chọn không gây vẽ lại cả trang. Các phím khác được bỏ qua.

## Luồng hoạt động

App cập nhật UI_Model từ DHT20, chạy LED và quét phím độc lập với trang.
UI_HandleKey tìm phím trong bảng screens[] tại Src/ui.c rồi mở trang.
UI_DataChanged chỉ gọi render của trang hiện tại. Vì vậy số đo mới hoặc
lỗi cảm biến không ghi đè lên bảng chữ cái. Trở về trang 0 sẽ thấy dữ liệu
mới nhất, hoặc dấu gạch đỏ nếu lần đo gần nhất lỗi.

Tất cả màn hình dùng cùng 6 hàng, tối đa 13 ký tự ASCII mỗi hàng, font
16x28 và nền đen. UI_Row(row, text, color) nhận row từ 0 đến 5.
Khi đổi trang, UI hủy job vẽ cũ và thay nội dung tất cả các hàng. TFT tiếp
tục gửi từng dòng nhỏ qua TFT_Process(), không xóa toàn màn hình bằng
hàm chặn. Trong lúc chuyển có thể thấy các hàng thay dần; không có cam
kết đổi toàn trang đồng thời hay chống tearing phần cứng.

## Sửa chức năng cũ

Sửa hàm render() trong file screen tương ứng. Không sửa main.c, driver
TFT hoặc bảng keymap. Hai trang 8/A hiện chỉ là mẫu vì chưa chốt chức năng.
Font chưa hỗ trợ tiếng Việt có dấu; dùng nhãn ASCII trên màn hình.

## Thêm chức năng mới

1. Tạo Src/screen_example.c theo mẫu dưới.
2. Thêm `extern const UI_Screen screen_example;` vào Inc/ui_screen.h.
3. Thêm `&screen_example,` vào screens[] trong Src/ui.c; chọn phím duy nhất.
4. Thêm file .c vào target_sources trong CMakeLists.txt gốc.
5. Build lại. Phần tử đầu screens[] là trang mặc định.

```c
#include "ui_screen.h"
#include "tft.h"
static void render(const UI_Model *model)
{
    (void)model;
    UI_Row(0, "NEW FUNCTION", TFT_GREEN);
    UI_Row(1, "YOUR CONTENT", TFT_WHITE);
    UI_Row(5, "0:HOME", TFT_WHITE);
}
const UI_Screen screen_example = {'1', render};
```

Nếu vẫn chỉ muốn bốn nút, thay nội dung một trong bốn trang hiện có thay
vì gán nút thứ năm. Màn mới có thể chỉ ghi các hàng cần dùng: bộ quản lý
đã đưa các hàng khác về trống khi mở trang.

Nếu chức năng mới có dữ liệu hoặc tính toán riêng, thêm dữ liệu vào
UI_Model, chạy tác vụ ngắn trong app rồi gọi UI_DataChanged khi dữ liệu
thay đổi. Không đặt HAL_Delay, vòng chờ hoặc đọc cảm biến trong render().
Render được gọi khi mở trang và khi model đổi; TFT tự bỏ qua nội dung
không đổi. Không tự ghi TFT từ tác vụ cảm biến.

## Kiểm tra

```
python3 tests/test_app_scheduler.py
python3 tests/test_tft_fields.py
cmake --preset Debug
cmake --build --preset Debug
```

Trên board: thử 0 → 4 → 8 → A → 0, bấm nhanh khi đang chuyển trang,
đợi DHT20 cập nhật khi đang xem ABC, tháo cảm biến rồi trở về trang 0.
Kiểm tra LED vẫn nhấp nháy ở mọi trang. Test host không xác nhận dây nối
hay chất lượng hiển thị thực tế trên board.
