/*
 * File:    main.c
 * Project: 03-countdown
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位数码管秒表示例（从 0 每秒加1，达到 99999999 后停止）
 *
 * 资源占用：
 *   定时器0 → common/smg  数码管动态扫描
 *   定时器1 → main.c      1秒定时（50ms × 20）
 */

#include <smg.h>

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
    /* EA 已在 Smg_Init() 中开启，不需要重复 */
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
    /* 使能控制 */
    P34 = 0; // led 关
    P35 = 1; // 点阵显示 关
    P36 = 0; // 数码管 开（138使能 + DIG8释放 + 245 DIR有效）

    unsigned long counter = 0;

    Smg_Init();
    Timer1_Init();

    Smg_ShowUInt(counter);

    while (1) {
        if (sec_flag) {
            sec_flag = 0;

            if (counter < 99999999UL) {
                counter++;
                Smg_ShowUInt(counter);
            }
        }
    }
}
