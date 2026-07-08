/*
 * File:    main.c
 * Project: 09-oled
 * MCU:     STC89C52RC
 * Brief:   OLED 初始化 + 清屏测试
 */
#include <stc89c52rc.h>
#include <delay.h>
#include <i2c.h>
#include <oled.h>

void main(void)
{
    OLED_Init();
    OLED_Clear();

    // 页寻址设置命令
    unsigned char cmd[] = {
       OLED_ADDR_MODE_VERTICAL, // 垂直寻址模式
       OLED_COM_PINS_SEQ, // 顺序寻址模式
       OLED_CMD_PAGE_ADDR, // 第一页
       OLED_CMD_COL_LOW, // 列低字节寻址命令
       OLED_CMD_COL_HIGH // 列高字节寻址命令
    };
    OLED_WriteCommand(cmd, 5);

    char date_bytes[16] = {0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00,
                    0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00};

    // 显示的内容
    OLED_WriteData(date_bytes,8);

    // 页寻址设置命令
    unsigned char cmd2[] = {
       OLED_ADDR_MODE_VERTICAL, // 垂直寻址模式
       OLED_COM_PINS_SEQ, // 顺序寻址模式
       OLED_CMD_PAGE_ADDR+1, // 页寻址命令
       OLED_CMD_COL_LOW, // 列低字节寻址命令
       OLED_CMD_COL_HIGH // 列高字节寻址命令
    };
    OLED_WriteCommand(cmd2, 5);

    // 显示的内容
    OLED_WriteData(date_bytes + 8,8);


    // 显示的内容
    OLED_WriteDataToRegion(2, 1, date_bytes);

    while (1) {
        /* 主循环空闲 */
    }
}
