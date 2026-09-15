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

Mọi thành phần chữ dùng tọa độ pixel (x, y), gốc (0, 0) ở trên trái.
X tăng sang phải, Y tăng xuống dưới, màn hình 240x240 ở hướng xoay hiện tại.
`UI_Text(id, x, y, text, color)` đặt hoặc cập nhật một thành phần.
ID 0–15 là mã thành phần, không phải số hàng. Mỗi thành phần chứa tối đa
31 ký tự ASCII, font 16x28; phần vượt cạnh phải/dưới được cắt.
Ví dụ hai thành phần trên cùng một dòng:

```c
UI_Text(0, 8, 40, "TEMP", TFT_WHITE);
UI_Text(1, 100, 40, "25.6 C", TFT_GREEN);
```

Dùng cùng ID và tọa độ mới để di chuyển. Truyền chuỗi rỗng hoặc gọi
TFT_RemoveText(id) để xóa. Chữ có nền trong suốt trên nền cảnh đen;
ID lớn hơn được vẽ trên ID nhỏ hơn khi chồng nhau. Bộ vẽ dựng lại cả dòng
bị thay đổi nên tự xóa vùng cũ khi di chuyển/rút ngắn chữ và phục hồi chữ
nằm dưới. Mỗi TFT_Process gửi tối đa một dòng 240 pixel (480 byte).

Đổi trang gọi TFT_ClearScene rồi khai báo các thành phần của trang mới.
Việc xóa/vẽ diễn ra từng dòng, không chặn cả trang và không đảm bảo đổi
trang đồng thời. Không trộn hàm vẽ trực tiếp TFT_FillRect/DrawString cũ
vào vùng do bộ quản lý cảnh sở hữu: dòng được dựng lại từ các thành phần
chữ sẽ ghi đè nội dung vẽ trực tiếp.

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
    UI_Text(0, 8, 12, "NEW FUNCTION", TFT_GREEN);
    UI_Text(1, 24, 80, "YOUR CONTENT", TFT_WHITE);
    UI_Text(2, 8, 208, "0:HOME", TFT_WHITE);
}
const UI_Screen screen_example = {'1', render};
```

Nếu vẫn chỉ muốn bốn nút, thay nội dung một trong bốn trang hiện có thay
vì gán nút thứ năm. Màn mới chỉ khai báo các thành phần cần dùng; cảnh cũ được xóa khi mở trang.

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
