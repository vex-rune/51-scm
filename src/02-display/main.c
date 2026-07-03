/*
 * File:    main.c
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位数码管主程序
 *
 * 功能：显示数字 12345678，1秒后变化
 */

#include "display.h"

/* 延时函数声明（复用 01-LED-Blink 的 delay.c） */
extern void DelayMs(unsigned int ms);

void main(void)
{
    /* 初始化显示驱动 */
    Display_Init();

    /* 自检 */
    Display_SelfTest();
    DelayMs(1000);

    /* 清屏 */
    Display_Clear();
    DelayMs(500);

    while (1) {
        /* 显示数字 12345678 */
        Display_ShowUInt(12345678UL);
        DelayMs(1000);

        /* 显示数字 -9999 */
        Display_ShowInt(-9999);
        DelayMs(1000);

        /* 显示 HEX: A5E7 */
        Display_SetDigit(4, 0x0A);   /* A */
        Display_SetDigit(5, 0x05);   /* 5 */
        Display_SetDigit(6, 0x0E);   /* E */
        Display_SetDigit(7, 0x07);   /* 7 */
        DelayMs(1000);

        /* 滚动显示 0~99999999 */
        {
            unsigned long n;
            for (n = 0; n <= 99999999UL; n += 11111UL) {
                Display_ShowUInt(n);
                DelayMs(200);
            }
        }
    }
}