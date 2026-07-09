/*
 * File:    oled.h
 * Project: 公共模块 - SSD1315 OLED 显示屏驱动
 * MCU:     STC89C52RC
 * Brief:   基于 i2c.c 的 SSD1315 OLED 高层驱动
 *
 * 提供所有 SSD1315 命令、寻址模式等的常量定义
 *
 * 依赖: i2c.c
 */

#ifndef __OLED_H__
#define __OLED_H__

/* ============================================================
 * I2C 地址
 * ============================================================ */

/* SSD1315 7位从机地址: 0x3C, 写地址 = 0x3C<<1 = 0x78 */
#define OLED_I2C_ADDR         0x78

/* 控制字节: 紧接的字节是命令或数据 */
#define OLED_CTRL_CMD         0x00
#define OLED_CTRL_DATA        0x40

/* ============================================================
 * 基础命令 (Fundamental Commands)
 * ============================================================ */

/* 设置对比度 (后接 1 字节, 0x00~0xFF) */
#define OLED_CMD_CONTRAST     0x81

/* 整屏显示模式 (A4=跟随RAM, A5=强制全亮) */
#define OLED_CMD_ENTIRE_ON    0xA4
#define OLED_CMD_ENTIRE_FORCE 0xA5

/* 正常/反色显示 */
#define OLED_CMD_NORMAL       0xA6
#define OLED_CMD_INVERSE      0xA7

/* 开关显示 */
#define OLED_CMD_DISPLAY_OFF  0xAE
#define OLED_CMD_DISPLAY_ON   0xAF

/* ============================================================
 * 滚动命令 (未实现, 仅列出)
 * ============================================================ */

#define OLED_CMD_RIGHT_HOR_SCROLL      0x26
#define OLED_CMD_LEFT_HOR_SCROLL       0x27
#define OLED_CMD_VERT_RIGHT_HOR_SCROLL 0x29
#define OLED_CMD_VERT_LEFT_HOR_SCROLL  0x2A
#define OLED_CMD_DEACTIVATE_SCROLL     0x2E
#define OLED_CMD_ACTIVATE_SCROLL       0x2F
#define OLED_CMD_SET_VERTICAL_AREA     0xA3

/* ============================================================
 * 地址设置命令
 * ============================================================ */

/* 设置列地址低 4 位 (0x00 | lowNibble) */
#define OLED_CMD_COL_LOW       0x00

/* 设置列地址高 4 位 (0x10 | highNibble) */
#define OLED_CMD_COL_HIGH      0x10

/* 设置页地址 (0xB0 | page, page 0~7) */
#define OLED_CMD_PAGE_ADDR     0xB0

/* 设置显示起始行 (0x40 | line, line 0~63) */
#define OLED_CMD_START_LINE    0x40

/* ============================================================
 * 硬件配置命令
 * ============================================================ */

/* 设置寻址模式 (后接 1 字节模式值) */
#define OLED_CMD_ADDR_MODE     0x20

/* 寻址模式参数 */
#define OLED_ADDR_MODE_PAGE       0x00   /* 页寻址 (默认) */
#define OLED_ADDR_MODE_HORIZONTAL 0x01   /* 水平寻址 */
#define OLED_ADDR_MODE_VERTICAL   0x02   /* 垂直寻址 */

/* 设置列地址范围 (水平/垂直模式, 后接 起始列, 结束列) */
#define OLED_CMD_COL_RANGE     0x21

/* 设置页地址范围 (水平/垂直模式, 后接 起始页, 结束页) */
#define OLED_CMD_PAGE_RANGE    0x22

/* Segment 重映射 (0xA0=反置, 0xA1=正常) */
#define OLED_CMD_SEG_REMAP_NORM  0xA1
#define OLED_CMD_SEG_REMAP_REV   0xA0

/* COM 输出扫描方向 (0xC0=正常, 0xC8=反置) */
#define OLED_CMD_COM_SCAN_NORM  0xC8
#define OLED_CMD_COM_SCAN_REV   0xC0

/* 设置多路复用率 (后接 1 字节, 1~64, 64 行屏用 0x3F) */
#define OLED_CMD_MUX_RATIO     0xA8
#define OLED_MUX_64            0x3F
#define OLED_MUX_32            0x1F

/* 设置 COM 引脚硬件配置 (后接 1 字节) */
#define OLED_CMD_COM_PINS      0xDA
#define OLED_COM_PINS_SEQ      0x02   /* 顺序 COM */
#define OLED_COM_PINS_ALT      0x12   /* 交替 COM (推荐) */

/* 设置显示偏移 (后接 1 字节 0~63) */
#define OLED_CMD_OFFSET        0xD3
#define OLED_OFFSET_0          0x00

/* 设置时钟分频/振荡器频率 (后接 1 字节, 高4位频率, 低4位分频) */
#define OLED_CMD_CLOCK         0xD5
#define OLED_CLOCK_DEFAULT     0x80

/* 设置预充电周期 (后接 1 字节) */
#define OLED_CMD_PRECHARGE     0xD9
#define OLED_PRECHARGE_DEFAULT 0xF1

/* 设置 VCOMH 电压 (后接 1 字节 0x00~0x07) */
#define OLED_CMD_VCOMH         0xDB
#define OLED_VCOMH_DEFAULT     0x40

/* 电荷泵设置 (后接 1 字节: 0x10=关, 0x14=开) */
#define OLED_CMD_CHARGEPUMP    0x8D
#define OLED_CHARGEPUMP_OFF    0x10
#define OLED_CHARGEPUMP_ON     0x14

/* NOP 命令 */
#define OLED_CMD_NOP           0xE3

/* ============================================================
 * 常用参数值
 * ============================================================ */

#define OLED_CONTRAST_DEFAULT  0xCF

/* ============================================================
 * 显示尺寸
 * ============================================================ */

#define OLED_WIDTH             128
#define OLED_HEIGHT            64
#define OLED_PAGES             (OLED_HEIGHT / 8)   /* 8 */

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief  初始化 SSD1315
 */
void OLED_Init(void);

/**
 * @brief  清空整个屏幕
 */
void OLED_Clear(void);

/**
 * @brief  向 SSD1315 发送多个命令
 * @param  command 命令数组
 * @param  length  命令个数
 */
void OLED_WriteCommand(const unsigned char *command, unsigned char length);

/**
 * @brief  向 SSD1315 发送多个数据
 * @param  data   数据数组
 * @param  length 数据个数
 */
void OLED_WriteData(const unsigned char *data, unsigned char length);

/**
 * @brief  写入字模数据 (每字节做位反转, 适配 SSD1315 bit0=顶行)
 * @note   字库采用 bit7=顶行 约定, 与 SSD1315 GDDRAM (bit0=顶行) 相反.
 *         绘制字符/图形等需要"高位在上"的字模时使用此函数.
 */
void OLED_WriteGlyphData(const unsigned char *data, unsigned char length);

/**
 * @brief  设置当前页和列地址（页寻址模式）
 * @param  page  页索引 (0-7)
 * @param  col   列起始位置 (0-127)
 */
void OLED_SetPage(unsigned char page, unsigned char col);

/**
 * @brief  在指定页写入 2 页内容 (256 字节, 自动拆分到 page 和 page+1)
 * @param  page  起始页 (0-6, 因为要写 2 页所以最大 6)
 * @param  col   列起始位置 (0-127)
 * @param  data  数据数组 (256 字节: data[0..127] 写 page, data[128..255] 写 page+1)
 */
void OLED_WriteDataToRegion(unsigned char page, unsigned char col, const unsigned char *data);

/* ============================================================
 * ASCII 字模 (8x16, 1 字节/列, 高位在上)
 * ============================================================
 *
 * 字符集: ASCII 0x20 (' ') 至 0x7E ('~') (共 95 字符)
 * 尺寸:   8 列宽 × 16 行高, 每个字符 16 字节, 跨两页显示
 * 取模:   段列式 - 每字节对应一列 8 行像素, bit7=顶行, bit0=底行
 *         data[0..7]  -> 上半页 (page)
 *         data[8..15] -> 下半页 (page+1)
 * 索引:   使用 OLED_AsciiIndex() 取得字符在字模表中的下标
 */
#define OLED_ASCII_WIDTH   8
#define OLED_ASCII_HEIGHT  16
#define OLED_ASCII_BYTES   16

/**
 * @brief  把字符映射到 OledAscii 表中的下标
 * @return 0..94 对应 ASCII 0x20..0x7E; 不在范围内返回 -1
 */
int OLED_AsciiIndex(char c);

/**
 * @brief  在指定 (page, col) 绘制单个 ASCII 字符 (8x16)
 * @param  page  起始页 (0-6, 因为字符高 16 行跨两页)
 * @param  col   列起始位置 (0-127)
 * @param  ch    待绘制的字符 (不在字模表内则不绘制)
 */
void OLED_ShowAsciiAt(unsigned char page, unsigned char col, char ch);

/**
 * @brief  在指定 (page, col) 显示以 '\0' 结尾的字符串
 * @note   不支持换行, 越界自动截断; 不在字模表内的字符会被跳过
 */
void OLED_ShowString(unsigned char page, unsigned char col, const char *str);

#endif /* __OLED_H__ */
