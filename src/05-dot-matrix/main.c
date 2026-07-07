/*
 * File:    main.c
 * Project: 05-dot-matrix
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8x8 LED 点阵演示（心形）
 */

#include <stc89c52rc.h>
#include "matrix.h"
#include "timer.h"

// 8x8 点阵图案集 (0=灭, 1=亮)
// 每行 8 位，从高位到低位

// 心形
static const unsigned char PIC_HEART[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000
};

// 笑脸
static const unsigned char PIC_SMILE[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100
};

// 数字 0
static const unsigned char PIC_0[8] = {
    0b00111100,
    0b01100110,
    0b01101110,
    0b01110110,
    0b01110110,
    0b01101110,
    0b01100110,
    0b00111100
};

// 数字 1
static const unsigned char PIC_1[8] = {
    0b00011000,
    0b00111000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b01111110,
    0b00000000
};

// 数字 2
static const unsigned char PIC_2[8] = {
    0b00111100,
    0b01100110,
    0b00000110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b01111110,
    0b00000000
};

// 数字 3
static const unsigned char PIC_3[8] = {
    0b00111100,
    0b01100110,
    0b00000110,
    0b00011100,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00000000
};

// 数字 4
static const unsigned char PIC_4[8] = {
    0b00001100,
    0b00011100,
    0b00101100,
    0b01001100,
    0b11111111,
    0b00001100,
    0b00001100,
    0b00000000
};

// 数字 5
static const unsigned char PIC_5[8] = {
    0b01111110,
    0b01100000,
    0b01111100,
    0b00000110,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00000000
};

// 数字 6
static const unsigned char PIC_6[8] = {
    0b00111100,
    0b01100110,
    0b01100000,
    0b01111100,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};

// 数字 7
static const unsigned char PIC_7[8] = {
    0b01111111,
    0b00000110,
    0b00001100,
    0b00011000,
    0b00011000,
    0b00110000,
    0b00110000,
    0b00000000
};

// 数字 8
static const unsigned char PIC_8[8] = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b00111100,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};

// 数字 9
static const unsigned char PIC_9[8] = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b00111110,
    0b00000110,
    0b00000110,
    0b00111100,
    0b00000000
};

// 字母 A
static const unsigned char PIC_A[8] = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b01111110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b00000000
};

// 字母 B
static const unsigned char PIC_B[8] = {
    0b01111100,
    0b01100110,
    0b01100110,
    0b01111100,
    0b01100110,
    0b01100110,
    0b01111100,
    0b00000000
};

// 字母 C
static const unsigned char PIC_C[8] = {
    0b00111100,
    0b01100110,
    0b01100000,
    0b01100000,
    0b01100000,
    0b01100110,
    0b00111100,
    0b00000000
};

// 字母 D
static const unsigned char PIC_D[8] = {
    0b01111100,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01111100,
    0b00000000
};

// 字母 E
static const unsigned char PIC_E[8] = {
    0b01111110,
    0b01100000,
    0b01100000,
    0b01111100,
    0b01100000,
    0b01100000,
    0b01111110,
    0b00000000
};

// 字母 F
static const unsigned char PIC_F[8] = {
    0b01111110,
    0b01100000,
    0b01100000,
    0b01111100,
    0b01100000,
    0b01100000,
    0b01100000,
    0b00000000
};

// 图案指针数组
static const unsigned char *PICS[] = {
    PIC_HEART, PIC_SMILE,
    PIC_0, PIC_1, PIC_2, PIC_3, PIC_4,
    PIC_5, PIC_6, PIC_7, PIC_8, PIC_9,
    PIC_A, PIC_B, PIC_C, PIC_D, PIC_E, PIC_F
};
#define PIC_COUNT (sizeof(PICS) / sizeof(PICS[0]))

// 当前显示的图案索引
static unsigned char current_pic = 0;
static unsigned int switch_counter = 0;
#define SWITCH_INTERVAL 4000  // 切换间隔（500us * 4000 = 2秒）

// 切换图案任务
void Switch_Pic(void) {
    switch_counter++;
    if (switch_counter >= SWITCH_INTERVAL) {
        switch_counter = 0;
        current_pic++;
        if (current_pic >= PIC_COUNT) {
            current_pic = 0;
        }
        Matrix_SetBuffer(PICS[current_pic]);
    }
}

void main(void)
{
    // 初始化
    Matrix_Init();
    FLIP_HORIZONTAL = 1;
    FLIP_VERTICAL = 1;
    ROTATE = 0;

    Matrix_SetBuffer(PICS[0]);
    Timer_Init();
    Timer_Register(Matrix_Scan);
    Timer_Register(Switch_Pic);  // 每隔 2 秒切换图案

    while (1) {
        // 主循环空闲，扫描由 Timer 中断驱动
    }
}
