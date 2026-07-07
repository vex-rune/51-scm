/*
 * File:    timer.h
 * Project: 公共模块 - 软件定时器调度
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   基于定时器0的任务调度器（函数指针数组）
 *
 * 用法：
 *   1. 调用 Timer_Init() 初始化（占用 T0，TIMER_TICK_US 中断）
 *   2. 调用 Timer_Register(任务函数) 注册任务
 *   3. 用 Timer_Enable / Timer_Disable 启停
 *
 * 注意：
 *   - 任务函数必须极短（设标志位、累加计数），禁止 DelayMs
 *   - Timer 模块与 smg 模块都使用 T0，**不能在同一项目同时使用**
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stc89c52rc.h>

/* ============================================================
 * 可调参数
 * ============================================================ */
#define TIMER_MAX_TASKS  8        /* 最多同时注册的任务数 */
#define TIMER_TICK_US    500      /* 调度周期（us），500us=2kHz */

/* 任务 ID：0 ~ TIMER_MAX_TASKS-1 为合法值，0xFF 表示无效 */
#define TIMER_INVALID_ID  0xFF

/* ============================================================
 * 类型定义
 * ============================================================ */

/* 回调函数类型：无参数无返回值 */
typedef void (*Timer_Callback)(void);

/* 任务结构体 */
typedef struct {
    Timer_Callback func;     /* 任务函数指针 */
    unsigned char  enabled;  /* 1=运行，0=暂停 */
} Timer_Task;

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief  初始化定时器0（模式1，TIMER_TICK_US 中断）+ 清空任务表
 * @note   调用后 EA=1 开启总中断
 */
void Timer_Init(void);

/**
 * @brief  注册一个任务
 * @param  func  任务函数指针
 * @return 任务 ID（0~7），任务表满返回 TIMER_INVALID_ID(0xFF)
 * @note   注册后默认启用
 */
unsigned char Timer_Register(Timer_Callback func);

/**
 * @brief  启动指定任务
 * @param  id  任务 ID（Timer_Register 的返回值）
 */
void Timer_Enable(unsigned char id);

/**
 * @brief  暂停指定任务
 * @param  id  任务 ID
 */
void Timer_Disable(unsigned char id);

/**
 * @brief  从任务表中移除（清空该槽位）
 * @param  id  任务 ID
 */
void Timer_Unregister(unsigned char id);

/**
 * @brief  定时器0中断服务程序
 * @note   不应被用户直接调用
 */
void Timer0_ISR(void) __interrupt(1);

#endif /* __TIMER_H__ */
