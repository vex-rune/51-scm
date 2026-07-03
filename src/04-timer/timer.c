/*
 * File:    timer.c
 * Project: 04-timer
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   定时器模块封装实现
 *
 * 内部机制：
 *   - 维护一个 8 项的任务表 tasks[]
 *   - T0 每 2ms 触发一次中断
 *   - ISR 中遍历 tasks[]，调用所有 enabled 的任务函数
 */

#include "timer.h"

/* ============================================================
 * 内部存储
 * ============================================================ */

/* 任务表（静态分配，省 RAM） */
static Timer_Task tasks[TIMER_MAX_TASKS];

/* 已注册任务数（仅用于遍历时减少循环次数） */
static unsigned char task_count = 0;

/* ============================================================
 * 初始化
 * ============================================================ */
void Timer_Init(void)
{
    unsigned char i;

    /* 清空任务表 */
    for (i = 0; i < TIMER_MAX_TASKS; i++) {
        tasks[i].func = 0;
        tasks[i].enabled = 0;
    }
    task_count = 0;

    /* 定时器0：模式1，2ms @ 11.0592MHz */
    /* 计数次数 = 2ms / (12/11059200) = 18432 */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = (65536 - 18432) / 256;
    TL0 = (65536 - 18432) % 256;
    ET0 = 1;
    TR0 = 1;
    EA  = 1;
}

/* ============================================================
 * 任务管理
 * ============================================================ */
unsigned char Timer_Register(Timer_Callback func)
{
    unsigned char i;

    if (func == 0) {
        return TIMER_INVALID_ID;
    }

    /* 找一个空槽 */
    for (i = 0; i < TIMER_MAX_TASKS; i++) {
        if (tasks[i].func == 0) {
            tasks[i].func = func;
            tasks[i].enabled = 1;   /* 默认启用 */
            if (i >= task_count) {
                task_count = i + 1;
            }
            return i;
        }
    }

    /* 任务表满 */
    return TIMER_INVALID_ID;
}

void Timer_Enable(unsigned char id)
{
    if (id >= TIMER_MAX_TASKS) return;
    if (tasks[id].func != 0) {
        tasks[id].enabled = 1;
    }
}

void Timer_Disable(unsigned char id)
{
    if (id >= TIMER_MAX_TASKS) return;
    tasks[id].enabled = 0;
}

void Timer_Unregister(unsigned char id)
{
    if (id >= TIMER_MAX_TASKS) return;
    tasks[id].func = 0;
    tasks[id].enabled = 0;

    /* 收缩 task_count（如果最高位的任务被清掉） */
    if (id + 1 == task_count) {
        while (task_count > 0 && tasks[task_count - 1].func == 0) {
            task_count--;
        }
    }
}

/* ============================================================
 * T0 中断：轮询调用所有启用的任务
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    unsigned char i;

    /* 遍历任务表，调用启用的任务 */
    for (i = 0; i < task_count; i++) {
        if (tasks[i].func != 0 && tasks[i].enabled) {
            tasks[i].func();
        }
    }

    /* 重装 2ms 初值 */
    TH0 = (65536 - 18432) / 256;
    TL0 = (65536 - 18432) % 256;
}
