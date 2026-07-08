/*
 * File:    oled.c
 * Project: 09-oled
 * MCU:     STC89C52RC
 * Brief:   SSD1315 OLED 显示屏驱动实现
 *
 * 使用 oled.h 中定义的命令常量, 不再硬编码 0xXX 数字
 */

#include "oled.h"
#include <i2c.h>

/**
 * @brief  向 SSD1315 发送一个或多个命令
 */
void OLED_WriteCommand(const unsigned char *command, unsigned char length)
{
    unsigned char i;
    I2C_Start();
    I2C_SendByte(OLED_I2C_ADDR);
    I2C_RecvAck();
    I2C_SendByte(OLED_CTRL_CMD);
    I2C_RecvAck();
    for (i = 0; i < length; i++) {
        I2C_SendByte(command[i]);
        I2C_RecvAck();
    }
    I2C_Stop();
}

/**
 * @brief  向 SSD1315 发送一个或多个数据
 */
void OLED_WriteData(const unsigned char *data, unsigned char length)
{
    unsigned char i;
    I2C_Start();
    I2C_SendByte(OLED_I2C_ADDR);
    I2C_RecvAck();
    I2C_SendByte(OLED_CTRL_DATA);
    I2C_RecvAck();
    for (i = 0; i < length; i++) {
        I2C_SendByte(data[i]);
        I2C_RecvAck();
    }
    I2C_Stop();
}

/**
 * @brief  SSD1315 初始化
 * @note   64 行 PMOLED 推荐配置
 */
void OLED_Init(void)
{
    unsigned char cmds[] = {
        OLED_CMD_DISPLAY_OFF,        /* 关闭显示 */
        OLED_CMD_CLOCK, OLED_CLOCK_DEFAULT,         /* 时钟分频 */
        OLED_CMD_MUX_RATIO, OLED_MUX_64,            /* 1/64 duty */
        OLED_CMD_OFFSET, OLED_OFFSET_0,             /* 显示偏移 0 */
        OLED_CMD_START_LINE,                        /* 起始行 0 */
        OLED_CMD_CHARGEPUMP, OLED_CHARGEPUMP_ON,     /* 启用电荷泵 */
        OLED_CMD_ADDR_MODE, OLED_ADDR_MODE_PAGE,    /* 页寻址模式 */
        OLED_CMD_SEG_REMAP_NORM,                     /* 列重映射 */
        OLED_CMD_COM_SCAN_NORM,                      /* 行重映射 */
        OLED_CMD_COM_PINS, OLED_COM_PINS_ALT,        /* 交替 COM */
        OLED_CMD_CONTRAST, OLED_CONTRAST_DEFAULT,   /* 对比度 */
        OLED_CMD_PRECHARGE, OLED_PRECHARGE_DEFAULT,  /* 预充电 */
        OLED_CMD_VCOMH, OLED_VCOMH_DEFAULT,          /* VCOMH */
        OLED_CMD_ENTIRE_ON,                          /* 跟随 RAM */
        OLED_CMD_NORMAL,                             /* 正常显示 */
        OLED_CMD_DISPLAY_ON                          /* 开启显示 */
    };
    OLED_WriteCommand(cmds, sizeof(cmds));
}

/**
 * @brief  清空整个屏幕
 */
void OLED_Clear(void)
{
    unsigned char page, col;
    unsigned char cmds[3];

    for (page = 0; page < OLED_PAGES; page++) {
        /* 设置页地址 + 列地址 0 */
        cmds[0] = OLED_CMD_PAGE_ADDR | page;
        cmds[1] = OLED_CMD_COL_LOW  | 0;   /* 列地址低 4 位 = 0 */
        cmds[2] = OLED_CMD_COL_HIGH | 0;   /* 列地址高 4 位 = 0 */
        OLED_WriteCommand(cmds, 3);

        /* 写 OLED_WIDTH 字节 0 到当前页 */
        I2C_Start();
        I2C_SendByte(OLED_I2C_ADDR);
        I2C_RecvAck();
        I2C_SendByte(OLED_CTRL_DATA);
        I2C_RecvAck();
        for (col = 0; col < OLED_WIDTH; col++) {
            I2C_SendByte(0x00);
            I2C_RecvAck();
        }
        I2C_Stop();
    }
}
