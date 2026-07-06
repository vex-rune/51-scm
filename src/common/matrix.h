/*
 * File:    matrix.h
 * Project: 公共模块 - 8x8 LED 点阵
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   共阴极 8x8 LED 点阵显示驱动（74HC595 驱动行 + P0 驱动列）
 *
 * 硬件连接：
 *   P0.0~P0.7  -> 段码输出（DZY-0~DZY-7 = 列, 共阴极）
 *   P1.0       -> 74HC595 SER  (pin 14, 串行数据输入)
 *   P1.1       -> 74HC595 SCK  (pin 11, 移位寄存器时钟)
 *   P1.2       -> 74HC595 RCK  (pin 12, 输出锁存器时钟)
 *   74HC595 QA~QH (pin 15, 1~7) -> 点阵 D0~D7（行, 阳极高=选中）
 *   P3.5       -> 点阵使能 DZ-EN（0=使能, 1=关闭）
 *
 * 74HC595 引脚 (DIP-16):
 *   15 QA   1 QB   2 QC   3 QD   4 QE   5 QF   6 QG   7 QH
 *    8 GND  9 QH' 10 SCLR 11 SCK 12 RCK 13  OE  14 SER 16 VCC
 *
 * 扫描方式: T0 每 MATRIX_SCAN_US 切换一行, 8行轮流
 *   消隐 -> 切行（595 锁存） -> 输出列
 *
 * 与 smg/timer 共享 T0 中断，**不能同时使用**
 */

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stc89c52rc.h>

/* ============================================================
 * 可调参数
 * ============================================================ */
#define MATRIX_ROWS      8
#define MATRIX_COLS      8
#define MATRIX_SCAN_US   1000  /* 单行扫描时长（us），8行一轮 = 8ms */

/* ============================================================
 * 硬件引脚配置（移植时修改这里）
 *   本头文件使用项目自定义 SFR 位名（P10/P11/P12），
 *   如改用 Keil 风格的 P1_0 需调整。
 * ============================================================ */
#define MATRIX_COL_PORT  P0   /* 列数据端口（P0.0~P0.7） */
#define MATRIX_SER       P10  /* 74HC595 SER  -> P1.0 */
#define MATRIX_SCK       P11  /* 74HC595 SCK  -> P1.1 */
#define MATRIX_RCK       P12  /* 74HC595 RCK  -> P1.2 */
#define MATRIX_EN        P35  /* 点阵使能（0=使能） */

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief  初始化点阵驱动（IO + T0 MATRIX_SCAN_US 扫描 + 使能）
 */
void Matrix_Init(void);

/**
 * @brief  显示一幅 8x8 图案
 * @param  pic  8 字节数组, 每字节代表一行（bit=1 表示 LED 亮）
 */
void Matrix_Show(const unsigned char pic[8]);

/**
 * @brief  清除显示（全部 LED 灭）
 */
void Matrix_Clear(void);

/**
 * @brief  自检（点亮全部 LED）
 */
void Matrix_SelfTest(void);

/**
 * @brief  播放字模数组的滚动效果（从右往左）
 * @param  font         字模数据, 8xN 字节, N 个字符, 每个字符 8 字节
 * @param  char_count   字模中的字符数
 * @param  speed_ms     每步间隔（毫秒）, 越小越快
 * @note   阻塞函数, 调用期间点阵仍正常扫描
 *         字模格式: [c0r0, ..., c0r7, c1r0, ...] 顺序排列
 */
void Matrix_PlayScroll(const unsigned char __code *font,
                       unsigned char char_count,
                       unsigned int speed_ms);

#endif /* __MATRIX_H__ */
