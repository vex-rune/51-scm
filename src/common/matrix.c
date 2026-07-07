/*
 * File:    matrix.c
 * Project: 公共模块 - 8x8 LED 点阵驱动
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   74HC595 驱动 8x8 LED 点阵矩阵
 */

#include "matrix.h"

// 配置参数（默认值可在 Matrix_Init 前修改）
unsigned char FLIP_HORIZONTAL = 1;  // 左右翻转: 0=正常, 1=翻转
unsigned char FLIP_VERTICAL = 1;     // 上下翻转: 0=正常, 1=翻转
unsigned char ROTATE = 0;            // 旋转: 0=不变, 1=90度, 2=180度, 3=270度

// 点阵矩阵缓存
static unsigned char MATRIX_BUFFER[8];

// 临时缓存用于旋转后的图案
static unsigned char ROTATED_BUFFER[8];

// 当前帧
static unsigned char current_frame = 0;

// 8x8 位图旋转函数
static void Matrix_Rotate(const unsigned char *src, unsigned char *dst, unsigned char angle) {
    unsigned char x, y;
    unsigned char bit;

    for (y = 0; y < 8; y++) {
        dst[y] = 0;
        for (x = 0; x < 8; x++) {
            bit = 0;
            switch (angle % 4) {
                case 0: // 0度：不旋转
                    bit = (src[y] >> (7 - x)) & 1;
                    break;
                case 1: // 90度顺时针：(y,x) -> (7-x,y)
                    bit = (src[7 - x] >> (7 - y)) & 1;
                    break;
                case 2: // 180度
                    bit = (src[7 - y] >> x) & 1;
                    break;
                case 3: // 270度顺时针：(y,x) -> (x,7-y)
                    bit = (src[x] >> y) & 1;
                    break;
            }
            if (bit) {
                dst[y] |= (1 << (7 - x));
            }
        }
    }
}

void Matrix_Init(void) {
    // 配置参数默认值
    FLIP_HORIZONTAL = 1;  // 左右翻转
    FLIP_VERTICAL = 1;     // 上下翻转
    ROTATE = 0;            // 旋转

    // 引脚初始化
    MATRIX_EN = 0;   // 使能点阵矩阵
    MATRIX_SCK = 0;  // 位移寄存器时钟初始化
    MATRIX_RCK = 0;  // 储存时钟初始化
    MATRIX_SER = 0;  // 串行初始化
    MATRIX_ROW = 0xFF;  // ROW 全部阻塞(高电平)
}

void Matrix_SetBuffer(const unsigned char *buffer) {
    if (ROTATE != 0) {
        Matrix_Rotate(buffer, ROTATED_BUFFER, ROTATE);
        for (unsigned char i = 0; i < 8; i++) {
            MATRIX_BUFFER[i] = ROTATED_BUFFER[i];
        }
    } else {
        for (unsigned char i = 0; i < 8; i++) {
            MATRIX_BUFFER[i] = buffer[i];
        }
    }
}

static void Matrix_Bit(unsigned char bit) {
    // 写入一个位, 到, SER 中
    MATRIX_SER = bit;
    // 触发一次写入
    MATRIX_SCK = 1;
    DelayUs(1);
    MATRIX_SCK = 0;
}

// 一帧画面
static void Matrix_Frame(unsigned int frame) {

    // 得到帧数据
    unsigned char row = MATRIX_BUFFER[frame];

    // 清理所有的 ROW
    MATRIX_ROW = 0xFF;

    // 根据帧得到本次显示的行,其余的都是0
    if (FLIP_VERTICAL) {
        for (unsigned char i = 8; i > 0; i--) {
            if (i - 1 == frame) {
                Matrix_Bit(row != 0); // 求或, 得到本行是否该亮
            } else {
                Matrix_Bit(0);
            }
        }
    } else {
        for (unsigned char i = 0; i < 8; i++) {
            Matrix_Bit(row & (1 << i));
        }
    }

    // 触发一次写入
    MATRIX_RCK = 1;
    DelayUs(1);
    MATRIX_RCK = 0;

    if (FLIP_HORIZONTAL) {
        // 位反转，因为发送顺序改为从高位到低位
        unsigned char reversed = 0;
        for (unsigned char i = 0; i < 8; i++) {
            reversed = (reversed << 1) | ((row >> i) & 1);
        }
        row = reversed;
    }
    MATRIX_ROW = ~row; // 取反, 因为是纵向低电平选通
}

void Matrix_Scan(void) {
    // 只扫描当前帧，由 Timer 控制调用频率
    Matrix_Frame(current_frame);
    current_frame++;
    if (current_frame >= 8) {
        current_frame = 0;
    }
}
