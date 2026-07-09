/*
 * File:    matrix.h
 * Project: 公共模块 - 8x8 LED 点阵驱动
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   74HC595 驱动 8x8 LED 点阵矩阵 - 简化版
 */

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stc89c52rc.h>
#include "delay.h"

// 引脚定义
#ifndef MATRIX_EN
#define MATRIX_EN  P35   // 使能引脚
#endif
#ifndef MATRIX_SER
#define MATRIX_SER P10   // 串行输入
#endif
#ifndef MATRIX_RCK
#define MATRIX_RCK P11   // 储存时钟
#endif
#ifndef MATRIX_SCK
#define MATRIX_SCK P12   // 位移时钟
#endif
#ifndef MATRIX_ROW
#define MATRIX_ROW P0    // 行选通
#endif

// API
void Matrix_Init(void);
void Matrix_SetBuffer(const unsigned char *buffer);
void Matrix_SetBufferCode(const unsigned char __code *buffer);
void Matrix_Scan(void);

#endif /* __MATRIX_H__ */
