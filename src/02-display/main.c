/*
 * File:    main.c
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位共阴极数码管主程序
 *
 * 功能：数码管动态扫描显示 12345678
 */

#include "display.h"

/* 延时函数声明（复用 01-LED-Blink 的 delay.c） */
extern void DelayMs(unsigned int ms);

void main(void)
{
    /* 使能控制 */
    P34 = 0; // led 关
    P35 = 1; // 点阵显示 关
    P36 = 0; // 数码管 开（138使能 + DIG8释放 + 245 DIR有效）

    /* 初始化显示驱动（启动定时器0中断扫描） */
    Display_Init();

    /* 全量更新显示缓冲区 */
    Display_UpdateBuf("12345678");

    while (1) {
        /* 动态扫描由定时器0中断驱动，无需额外处理 */
        DelayMs(10);
    }
}
