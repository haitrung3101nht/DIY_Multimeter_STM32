/* Hardware transport: I2C1 configured by CubeMX. */
#include "board_oled.h"
#include "i2c.h"
/* Sửa: dùng hi2c1 do CubeMX khai báo, bỏ handle bus riêng của BSP. */

HAL_StatusTypeDef Board_OLED_Init(void)
{
    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        return HAL_ERROR;
    }
    /* Sửa: chỉ kiểm tra bus đã sẵn sàng; MX_I2C1_Init() trong main.c
       chịu trách nhiệm cấu hình GPIO, clock và I2C trước App_Init().
       Bỏ cấu hình và reset I2C1 tại BSP để tránh khởi tạo hai lần. */

    return HAL_I2C_IsDeviceReady(&hi2c1, BOARD_OLED_ADDRESS << 1, 2, 100);
    /* Sửa: kiểm tra OLED có ACK qua bus của CubeMX. Khi không có phản hồi,
       App sẽ thử lại sau một giây mà không tự cấu hình lại I2C1. */
}

HAL_StatusTypeDef Board_OLED_Write(uint8_t control, const uint8_t *data, uint16_t size)
{
    /* SSD1306 control byte is sent in the memory-address position. */
    return HAL_I2C_Mem_Write(&hi2c1, BOARD_OLED_ADDRESS << 1, control,
                            I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, size, 100);
    /* Sửa: truyền bằng hi2c1 thay cho bus riêng; địa chỉ OLED, control byte
       và timeout giữ nguyên. Mọi giao tiếp dùng chung handle của CubeMX. */
}
