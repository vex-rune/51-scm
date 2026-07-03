/*
 * File:    matrix.h
 * Project: 05-dot-matrix
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8x8 LED 点阵显示驱动 (74HC595 驱动行 + P3 驱动列)
 *
 * 硬件连接:
 *   P1.0 -> 74HC595 SER  (pin 14, 串行数据输入)
 *   P1.1 -> 74HC595 SCK  (pin 11, 移位寄存器时钟)
 *   P1.2 -> 74HC595 RCK  (pin 12, 输出锁存器时钟)
 *   74HC595 OE   (pin 13) -> GND  (使能)
 *   74HC595 SCLR (pin 10) -> VCC  (不复位)
 *   74HC595 QA~QH (pin 15, 1~7) -> 点阵 D0~D7 (行, 阳极)
 *   P3.0~P3.7                -> 点阵 C0~C7 (列, 阴极)
 *
 * 74HC595 引脚 (DIP-16):
 *   15 QA   1 QB   2 QC   3 QD   4 QE   5 QF   6 QG   7 QH
 *    8 GND  9 QH' 10 SCLR 11 SCK 12 RCK 13  OE  14 SER 16 VCC
 *
 * 点阵类型: 共阴极 (行高 + 列低 = LED 亮)
 * 扫描方式: T0 每 1ms 切换一行, 8ms 一帧 (~125Hz)
 */

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stc89c52rc.h>

/* ============================================================
 * 硬件接口定义
 * ============================================================ */

/* 74HC595 控制引脚 (P1.0 / P1.1 / P1.2) */
#define HC595_DATA   P1_0   /* SER, pin 14: 串行数据输入 */
#define HC595_CLK    P1_1   /* SCK, pin 11: 移位时钟    */
#define HC595_LATCH  P1_2   /* RCK, pin 12: 锁存时钟    */

/* 点阵尺寸 */
#define MATRIX_ROWS  8
#define MATRIX_COLS  8

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief  初始化点阵驱动 (IO + T0 1ms 扫描)
 */
void Matrix_Init(void);

/**
 * @brief  显示一幅 8x8 图案
 * @param  pic  8 字节数组, 每字节代表一行 (bit=1 表示 LED 亮)
 */
void Matrix_Show(const unsigned char pic[8]);

/**
 * @brief  清除显示 (全部 LED 灭)
 */
void Matrix_Clear(void);

/**
 * @brief  自检 (点亮全部 LED)
 */
void Matrix_SelfTest(void);

/**
 * @brief  播放多帧动画
 * @param  frames       帧数据 (frame_count * 8 字节, 连续排列)
 * @param  frame_count  帧数
 * @param  interval_ms  每帧间隔 (毫秒)
 * @note   阻塞函数, 调用期间点阵仍正常扫描
 */
void Matrix_PlayAnimation(const unsigned char *frames,
                          unsigned char frame_count,
                          unsigned int  interval_ms);

#endif /* __MATRIX_H__ */
