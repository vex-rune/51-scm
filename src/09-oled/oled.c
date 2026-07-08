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

/**
 * @brief  设置当前页和列地址（页寻址模式）
 * @param  page  页索引 (0-7), 对应屏幕纵向 8 个像素
 * @param  col   列索引 (0-127), 对应屏幕横向 1 个像素
 *
 * 原理说明:
 *   SSD1315 的 GDDRAM 按"页"组织, 每页 8 行像素, 共 8 页(64 行)
 *   设置页地址后, 后续写入的数据会自动填到该页对应列
 *
 *   命令格式(SSD1315 数据手册):
 *     - 页地址命令: 0xB0 | page
 *       例: page=0 -> 0xB0 (Page 0)
 *           page=1 -> 0xB1 (Page 1)
 *           page=7 -> 0xB7 (Page 7)
 *
 *     - 列地址低 4 位: 0x00 | (col & 0x0F)
 *       例: col=0x23 -> 0x03 (低 4 位 = 3)
 *           col=0x7F -> 0x0F (低 4 位 = F)
 *
 *     - 列地址高 4 位: 0x10 | ((col >> 4) & 0x0F)
 *       例: col=0x23 -> 0x12 (高 4 位 = 2, 命令 = 0x10|2 = 0x12)
 *           col=0x7F -> 0x17 (高 4 位 = 7, 命令 = 0x10|7 = 0x17)
 *
 *   ⚠️ 原代码存在 BUG: OLED_WriteCommand(cmd2, 5) 中 length 写成了 5
 *      但 cmd2 实际只有 3 个字节, 5 字节会越界读后面内存的随机数据
 *      已修正为 3
 */
void OLED_SetPage(unsigned char page, unsigned char col)
{
    unsigned char cmd[3];

    /* 1. 页地址命令: 0xB0 | page
     *    OR 运算: 用 0xB0 保留高 4 位, 低 4 位由 page 提供 */
    cmd[0] = OLED_CMD_PAGE_ADDR | (page & 0x07);  /* 限定 0-7 防止越界 */

    /* 2. 列地址低 4 位: 0x00 | (col & 0x0F)
     *    用 col & 0x0F 取出低 4 位(0-15)
     *    0x00 这里其实是 OLED_CMD_COL_LOW, 直接用宏更清晰 */
    cmd[1] = OLED_CMD_COL_LOW | (col & 0x0F);

    /* 3. 列地址高 4 位: 0x10 | (col >> 4)
     *    用 col >> 4 取出高 4 位
     *    0x10 = 0b00010000, 是 SSD1315 "设置列地址高 4 位" 的命令
     *    加 |(col>>4) 表示高 4 位的值(0-15)
     *    0x10 这里是 OLED_CMD_COL_HIGH */
    cmd[2] = OLED_CMD_COL_HIGH | ((col >> 4) & 0x0F);

    /* 发送 3 个命令字节到 SSD1315
     * ⚠️ 修复原 BUG: 原来是 5, 应该是 3 (cmd 数组只有 3 个元素) */
    OLED_WriteCommand(cmd, 3);
}

/**
 * @brief  在指定页写入 2 页内容 (256 字节, 自动拆分到 page 和 page+1)
 * @param  page  起始页 (0-6, 因为要写 2 页所以最大 6)
 * @param  col   列起始位置 (0-127)
 * @param  data  数据数组 (256 字节, 包含 page 和 page+1 两页的内容)
 *
 * 数据布局:
 *   data[0..127]   -> 写入 page
 *   data[128..255] -> 写入 page+1
 *
 * 流程:
 *   1. 写入第 1 页 (data[0..127]) 到 page
 *   2. 写入第 2 页 (data[128..255]) 到 page+1
 *   3. 每页写满 8 字节, 到达末尾后回到当前页第 0 列继续写
 */
void OLED_WriteDataToRegion(
    unsigned char page,
    unsigned char col,
    const unsigned char *data)
{

    unsigned char data_size = 16;
    // 自动拆分 data , 8个为一部分
    unsigned char i;
    for (i = 0; i < data_size/8; i++) {
        OLED_SetPage(page + i, col);
        OLED_WriteData(data + (i*8), 8);
    }



    // /* 第 1 部分: data[0..127] -> 写入 page */
    // OLED_SetPage(page, col);
    // OLED_WriteData(data, 8);

    // /* 第 2 部分: data[128..255] -> 写入 page+1 */
    // OLED_SetPage(page + 1, col);
    // OLED_WriteData(data + 8,8);
}
