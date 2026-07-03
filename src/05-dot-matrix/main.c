/*
 * File:    main.c
 * Project: 05-dot-matrix
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8x8 点阵图案演示
 *
 * 演示内容 (循环):
 *   1. 自检 (全亮 -> 全灭)
 *   2. 爱心
 *   3. 数字 0~3
 *   4. 笑脸
 *   5. 方框
 */

#include "matrix.h"

/* 软件延时 (粗略, ~1ms 每内循环 @ 11.0592MHz) */
static void DelayMs(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++) {
            ;
        }
    }
}

/* ============================================================
 * 取模数据 (8x8, 每字节代表一行, bit=1 表示 LED 亮)
 * ============================================================ */

/* 爱心 */
static const unsigned char PIC_HEART[8] = {
    0x00, 0x1C, 0x3E, 0x7E, 0x7E, 0x3E, 0x1C, 0x00
};

/* 数字 0 */
static const unsigned char PIC_0[8] = {
    0x00, 0x3E, 0x41, 0x41, 0x41, 0x41, 0x3E, 0x00
};

/* 数字 1 */
static const unsigned char PIC_1[8] = {
    0x00, 0x42, 0x42, 0x7F, 0x40, 0x40, 0x00, 0x00
};

/* 数字 2 */
static const unsigned char PIC_2[8] = {
    0x00, 0x62, 0x51, 0x49, 0x49, 0x49, 0x46, 0x00
};

/* 数字 3 */
static const unsigned char PIC_3[8] = {
    0x00, 0x22, 0x41, 0x49, 0x49, 0x49, 0x36, 0x00
};

/* 方框 (自检用) */
static const unsigned char PIC_BOX[8] = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
};

/* 笑脸 */
static const unsigned char PIC_SMILE[8] = {
    0x00, 0x00, 0x24, 0x00, 0x42, 0x3C, 0x00, 0x00
};

/* ============================================================
 * 主程序
 * ============================================================ */
void main(void)
{
    Matrix_Init();

    while (1) {
        /* 1. 自检 */
        Matrix_SelfTest();
        DelayMs(1000);
        Matrix_Clear();
        DelayMs(500);

        /* 2. 爱心 2 秒 */
        Matrix_Show(PIC_HEART);
        DelayMs(2000);

        /* 3. 数字 0~3, 各 1 秒 */
        Matrix_Show(PIC_0);
        DelayMs(1000);
        Matrix_Show(PIC_1);
        DelayMs(1000);
        Matrix_Show(PIC_2);
        DelayMs(1000);
        Matrix_Show(PIC_3);
        DelayMs(1000);

        /* 4. 笑脸 2 秒 */
        Matrix_Show(PIC_SMILE);
        DelayMs(2000);

        /* 5. 方框 1 秒 */
        Matrix_Show(PIC_BOX);
        DelayMs(1000);
    }
}
