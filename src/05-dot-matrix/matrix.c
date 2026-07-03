/*
 * File:    matrix.c
 * Project: 05-dot-matrix
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8x8 LED 点阵显示驱动实现
 *
 * 扫描原理:
 *   - 当前显示图案存于 frame_buf[8]
 *   - T0 每 1ms 中断一次, 切换到下一行
 *   - 行选通过 595 串行移位实现 (点亮行 = 0, 灭行 = 1)
 *   - 列数据直接送 P3 (共阴极: 0=亮, 1=灭)
 *
 * 74HC595 引脚分配 (DIP-16):
 *   QA (pin 15) -> D0 (第 0 行)
 *   QB (pin  1) -> D1 (第 1 行)
 *   ...
 *   QH (pin  7) -> D7 (第 7 行)
 *   QH' (pin 9)  未使用 (无级联)
 */

#include "matrix.h"

/* ============================================================
 * 当前显示的图案 (由 Matrix_Show 写入)
 * ============================================================ */
static unsigned char frame_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 当前扫描行 (0~7) */
static unsigned char cur_row = 0;

/* ============================================================
 * 扫描一行
 *   行选: 1 字节送 595, Q0~Q7 -> D0~D7
 *         bit=0 表示该行点亮 (阳极高)
 *         bit=1 表示该行熄灭
 *   列选: 直接写 P3, bit=0 表示该列点亮
 *         (共阴极: 行高 + 列低 = 该点亮)
 * ============================================================ */
static void ScanRow(unsigned char row)
{
    unsigned char i;
    unsigned char row_mask;
    unsigned char col_data;

    /* 行选字节: 当前行 = 0, 其他行 = 1 */
    row_mask = (unsigned char)~(0x01 << row);

    /* 列数据 (共阴极需取反) */
    col_data = (unsigned char)~frame_buf[row];

    /* 输出列数据 */
    P3 = col_data;

    /* 串行发送行选字节到 595, MSB 先发 */
    for (i = 0; i < 8; i++) {
        HC595_DATA = (row_mask & 0x80) ? 1 : 0;
        HC595_CLK = 0;
        row_mask <<= 1;
        HC595_CLK = 1;
    }

    /* 锁存: 上升沿把移位寄存器数据送输出 */
    HC595_LATCH = 0;
    HC595_LATCH = 1;
    HC595_LATCH = 0;
}

/* ============================================================
 * 初始化
 * ============================================================ */
void Matrix_Init(void)
{
    unsigned char i;

    /* 清空图案 */
    for (i = 0; i < 8; i++) {
        frame_buf[i] = 0;
    }
    cur_row = 0;

    /* 595 控制引脚初始电平 */
    HC595_DATA = 0;
    HC595_CLK  = 0;
    HC595_LATCH = 0;

    /* 全部列拉高 (全部灭) */
    P3 = 0xFF;

    /* T0: 模式1, 1ms @ 11.0592MHz */
    /* 计数次数 = 1ms / (12/11059200) = 9216 */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = (65536 - 9216) / 256;
    TL0 = (65536 - 9216) % 256;
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
    for (i = 0; i < 8; i++) {
        frame_buf[i] = pic[i];
    }
}

void Matrix_Clear(void)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        frame_buf[i] = 0;
    }
}

void Matrix_SelfTest(void)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        frame_buf[i] = 0xFF;
    }
}

/* ============================================================
 * 播放动画 (阻塞)
 * ============================================================ */
void Matrix_PlayAnimation(const unsigned char *frames,
                          unsigned char frame_count,
                          unsigned int  interval_ms)
{
    unsigned char i;
    unsigned int  j;
    unsigned char k;

    if (frames == 0 || frame_count == 0) return;

    for (k = 0; k < frame_count; k++) {
        Matrix_Show(&frames[k * 8]);

        /* 软件延时 (后台扫描照常进行) */
        for (j = 0; j < interval_ms; j++) {
            for (i = 0; i < 120; i++) {
                ;
            }
        }
    }
}

/* ============================================================
 * T0 中断: 每 1ms 扫描一行
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    ScanRow(cur_row);

    cur_row++;
    if (cur_row >= MATRIX_ROWS) {
        cur_row = 0;
    }

    TH0 = (65536 - 9216) / 256;
    TL0 = (65536 - 9216) % 256;
}
