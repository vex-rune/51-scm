/*
 * File:    main.c
 * Project: 03-countdown
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位数码管倒计时器
 *
 * 功能：从 99999999 开始每秒减 1，归零后停止。
 *       定时器0 -> 2ms 显示扫描（display.c）
 *       定时器1 -> 50ms×20 = 1秒（本文件）
 *
 * 硬件连接：
 *   P1.0 -> 74HC595 DATA (DS)
 *   P1.1 -> 74HC595 CLK  (SH_CP)
 *   P1.2 -> 74HC595 LATCH(ST_CP)
 *   P2.0~P2.7 -> 位选 PNP 三极管基极
 */

#include "display.h"

/* ============================================================
 * 1 秒定时器（定时器1，50ms × 20）
 * ============================================================ */

volatile unsigned char sec_flag = 0;       /* 1 秒到标志 */
volatile unsigned char count_50ms = 0;     /* 50ms 累加计数 */

/**
 * @brief  初始化定时器1（50ms 基间隔，叠加 20 次得 1 秒）
 * @note   机器周期 = 12/11059200 ≈ 1.085μs
 *         50ms 需要计数 46080 次 -> 初值 0x4C00
 */
void Timer1_Init(void)
{
    TMOD &= 0x0F;     /* 只清 T1 高 4 位，不动 T0 */
    TMOD |= 0x10;     /* T1 模式1: 16位定时器 */
    TH1 = 0x4C;
    TL1 = 0x00;
    ET1 = 1;
    TR1 = 1;
    /* EA 已在 Display_Init() 中开启，不需要重复 */
}

void Timer1_ISR(void) __interrupt(3)
{
    TH1 = 0x4C;
    TL1 = 0x00;

    count_50ms++;
    if (count_50ms >= 20) {
        count_50ms = 0;
        sec_flag = 1;
    }
}

/* ============================================================
 * 主程序
 * ============================================================ */
void main(void)
{
    unsigned long countdown = 99999999UL;

    Display_Init();
    Timer1_Init();

    Display_ShowUInt(countdown);

    while (1) {
        if (sec_flag) {
            sec_flag = 0;

            if (countdown > 0) {
                countdown--;
                Display_ShowUInt(countdown);
            }
        }
    }
}
