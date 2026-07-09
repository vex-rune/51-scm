/*
 * File:    matrix.c
 * Project: 公共模块 - 8x8 LED 点阵驱动
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   74HC595 驱动 8x8 LED 点阵矩阵 - 简化版
 */

#include "matrix.h"

// 点阵矩阵缓存
static unsigned char MATRIX_BUFFER[8];

// 当前扫描行
static unsigned char current_row = 0;

// 互斥控制标志（由 main.c 设置）
extern unsigned char g_matrix_enabled;

void Matrix_Init(void) {
    // 引脚初始化
    MATRIX_EN = 0;      // 使能点阵
    MATRIX_SCK = 0;     // 时钟低
    MATRIX_RCK = 0;     // 锁存低
    MATRIX_SER = 0;     // 数据低
    MATRIX_ROW = 0xFF;  // 所有行关闭 (高电平)
}

void Matrix_SetBuffer(const unsigned char *buffer) {
    for (unsigned char i = 0; i < 8; i++) {
        MATRIX_BUFFER[i] = buffer[i];
    }
}

void Matrix_SetBufferCode(const unsigned char __code *buffer) {
    for (unsigned char i = 0; i < 8; i++) {
        MATRIX_BUFFER[i] = buffer[i];
    }
}

// 向 595 发送一个字节
static void Matrix_SendByte(unsigned char dat) {
    for (unsigned char i = 0; i < 8; i++) {
        MATRIX_SER = (dat & 0x80) ? 1 : 0;  // 取最高位
        MATRIX_SCK = 1;
        MATRIX_SCK = 0;
        dat <<= 1;
    }
}

void Matrix_Scan(void) {
    // 互斥控制 - 只有点阵激活时才扫描
    if (!g_matrix_enabled) {
        return;
    }

    // 关闭当前行 (防止鬼影)
    MATRIX_ROW = 0xFF;

    // 发送列数据到 595
    unsigned char col_data = MATRIX_BUFFER[current_row];
    Matrix_SendByte(col_data);

    // 锁存数据
    MATRIX_RCK = 1;
    MATRIX_RCK = 0;

    // 打开当前行 (低电平有效)
    MATRIX_ROW = ~(1 << current_row);

    // 下一行
    current_row++;
    if (current_row >= 8) {
        current_row = 0;
    }
}
