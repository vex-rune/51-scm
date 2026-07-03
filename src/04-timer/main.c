/*
 * File:    main.c
 * Project: 04-timer
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   定时器模块封装示例
 *
 * 演示：注册 3 个任务，硬件自动每 2ms 轮询调用
 *   f1() — 500ms 翻转 P1.0 (LED)
 *   f2() — 1s    翻转 P1.1
 *   f3() — 2s    翻转 P1.2
 *
 * 硬件连接：LED 接 P1.0 / P1.1 / P1.2
 */

#include "timer.h"

/* ============================================================
 * 任务函数（必须短：设标志位或翻转引脚，不要 DelayMs）
 * ============================================================ */

static unsigned int cnt1 = 0;
static unsigned int cnt2 = 0;
static unsigned int cnt3 = 0;

void f1_500ms(void)
{
    if (++cnt1 >= 250) {   /* 250 × 2ms = 500ms */
        cnt1 = 0;
        P1_0 = !P1_0;
    }
}

void f2_1s(void)
{
    if (++cnt2 >= 500) {   /* 500 × 2ms = 1s */
        cnt2 = 0;
        P1_1 = !P1_1;
    }
}

void f3_2s(void)
{
    if (++cnt3 >= 1000) {  /* 1000 × 2ms = 2s */
        cnt3 = 0;
        P1_2 = !P1_2;
    }
}

/* ============================================================
 * 主程序
 * ============================================================ */
void main(void)
{
    /* 初始化定时器模块（启动 T0 中断，2ms 间隔） */
    Timer_Init();

    /* 注册 3 个任务 — 硬件会自动每 2ms 调用一次 */
    Timer_Register(f1_500ms);
    Timer_Register(f2_1s);
    Timer_Register(f3_2s);

    /* 主循环啥也不用干，3 个任务由 ISR 调度 */
    while (1) {
        /* 空闲 — 所有耗时操作都在中断里完成 */
    }
}
