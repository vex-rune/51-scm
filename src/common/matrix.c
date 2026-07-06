/*
 * File:    matrix.c
 * Project: 公共模块 - 8x8 LED 点阵
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   共阴极 8x8 LED 点阵显示驱动实现
 *
 * 扫描原理:
 *   - frame_buf[8] 存当前图案, 一字节一行, bit=1=亮
 *   - T0 每 MATRIX_SCAN_US 中断一次, 切换到下一行
 *   - 行选通过 595 串行移位（点亮行 = 0, 灭行 = 1）
 *   - 列数据直接送 MATRIX_COL_PORT（共阴极: 0=亮, 1=灭）
 *
 * 扫描时序（消隐 → 切行 → 列码）:
 *   1) MATRIX_COL_PORT = 0xFF         ← 消隐
 *   2) 串行发送 row_mask 到 595, 锁存
 *   3) MATRIX_COL_PORT = ~frame_buf[row] ← 输出新列码
 */

#include "matrix.h"

/* ============================================================
 * 内部存储
 * ============================================================ */
static unsigned char frame_buf[8];
static unsigned char cur_row = 0;

/* 扫描参数（由 matrix.h 宏自动计算） */
#define FOSC_HZ       11059200UL
#define T0_PRESCALER  12
#define SCAN_TICKS    ((unsigned int)(FOSC_HZ / 1000000UL * MATRIX_SCAN_US / T0_PRESCALER))
#define SCAN_RELOAD   (65536UL - SCAN_TICKS)

/* ============================================================
 * 扫描一行
 * ============================================================ */
static void ScanRow(unsigned char row)
{
    unsigned char i;
    unsigned char row_mask;
    unsigned char col_data;

    /* 行选: 1 = 该行点亮（阳极高），0 = 灭 */
    row_mask = (unsigned char)(0x01 << row);

    /* 列选: 0 = 该列点亮（阴极低），1 = 灭
     *   frame_buf[row] 的 bit=1 表示亮 -> 列输出低 = 0 */
    col_data = (unsigned char)~frame_buf[row];

    /* 1) 消隐（关闭列码，避免位选切换时撞到错位） */
    MATRIX_COL_PORT = 0xFF;

    /* 2) 串行发送 row_mask 到 595（MSB 先发） */
    for (i = 0; i < 8; i++) {
        MATRIX_SER = (row_mask & 0x80) ? 1 : 0;
        MATRIX_SCK = 0;
        row_mask <<= 1;
        MATRIX_SCK = 1;
    }
    /* 锁存 */
    MATRIX_RCK = 0;
    MATRIX_RCK = 1;
    MATRIX_RCK = 0;

    /* 3) 输出新列码（行选已稳定） */
    MATRIX_COL_PORT = col_data;
}

/* ============================================================
 * 初始化
 * ============================================================ */
void Matrix_Init(void)
{
    unsigned char i;

    /* 清空图案 */
    for (i = 0; i < MATRIX_ROWS; i++) {
        frame_buf[i] = 0;
    }
    cur_row = 0;

    /* 595 控制引脚初始电平 */
    MATRIX_SER = 0;
    MATRIX_SCK = 0;
    MATRIX_RCK = 0;

    /* 全部列拉高（全部灭） */
    MATRIX_COL_PORT = 0xFF;

    /* 点阵使能（低电平有效） */
    MATRIX_EN = 0;

    /* T0: 模式1, MATRIX_SCAN_US @ 11.0592MHz */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = (unsigned char)(SCAN_RELOAD >> 8);
    TL0 = (unsigned char)(SCAN_RELOAD & 0xFF);
    ET0 = 1;
    TR0 = 1;
    EA  = 1;
}

/* ============================================================
 * 公共 API
 * ============================================================ */
void Matrix_Show(const unsigned char pic[8])
{
    unsigned char i;
    for (i = 0; i < MATRIX_ROWS; i++) {
        frame_buf[i] = pic[i];
    }
}

void Matrix_Clear(void)
{
    unsigned char i;
    for (i = 0; i < MATRIX_ROWS; i++) {
        frame_buf[i] = 0;
    }
}

void Matrix_SelfTest(void)
{
    unsigned char i;
    for (i = 0; i < MATRIX_ROWS; i++) {
        frame_buf[i] = 0xFF;
    }
}

/* ============================================================
 * 滚动显示
 *   把字模数组拼成超宽图, 每次显示 8 列, offset 每次 +1
 * ============================================================ */

/* 简易软件延时（滚动用） */
static void ScrollDelayMs(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++) {
            ;
        }
    }
}

static unsigned char scroll_buf[8];

void Matrix_PlayScroll(const unsigned char __code *font,
                       unsigned char char_count,
                       unsigned int speed_ms)
{
    unsigned char i;
    unsigned int  step;
    unsigned char row;
    unsigned int  abs_col;
    unsigned int  char_pos;
    unsigned char char_col;
    const unsigned char __code *p;

    if (font == 0 || char_count == 0) return;

    /* 总步数 = 字模总列数 + 屏幕宽，让字符从右外滑到左外 */
    for (step = 0; step < (unsigned int)char_count * 8 + 8; step++) {
        /* 清空滚动缓冲区 */
        for (i = 0; i < MATRIX_ROWS; i++) {
            scroll_buf[i] = 0;
        }

        /* 拼出当前 8 列 */
        for (i = 0; i < MATRIX_COLS; i++) {
            abs_col = step + i;
            char_pos = abs_col >> 3;
            char_col = (unsigned char)(abs_col & 0x07);

            if (char_pos >= char_count) continue;

            p = &font[char_pos * 8];
            for (row = 0; row < MATRIX_ROWS; row++) {
                if (p[row] & (0x80 >> char_col)) {
                    scroll_buf[row] |= (unsigned char)(0x80 >> i);
                }
            }
        }

        /* 送显示 */
        Matrix_Show(scroll_buf);

        /* 延时一帧 */
        ScrollDelayMs(speed_ms);
    }
}

/* ============================================================
 * T0 中断：每 MATRIX_SCAN_US 扫描一行
 * ============================================================ */
void Matrix_Timer0_ISR(void) __interrupt(1)
{
    ScanRow(cur_row);

    cur_row++;
    if (cur_row >= MATRIX_ROWS) {
        cur_row = 0;
    }

    TH0 = (unsigned char)(SCAN_RELOAD >> 8);
    TL0 = (unsigned char)(SCAN_RELOAD & 0xFF);
}
