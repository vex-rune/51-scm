/*
 * File:    main.c
 * Project: 01-LED-Blink
 * MCU:     STC89C52RC (8051, 11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   点亮 P1.0 引脚上的 LED 并使其以约 1 Hz 频率闪烁
 *
 * 注意：SDCC 的 SFR 位命名规则与 Keil C51 相同（可直接用 P1_0 等），
 *       但需通过 --model-small / board config 保证存储模式一致。
 */

#include <stc89c52rc.h>

/* 延时函数声明 */
extern void DelayMs(unsigned int ms);

/* LED 接在 P1.0，低电平点亮（开发板典型接法） */
#define LED_PIN P1_0

void main(void)
{
    /* 关闭所有 LED 初始态 */
    P1 = 0xFF;

    while (1) {
        LED_PIN = 0;     /* 点亮 LED */
        DelayMs(500);
        LED_PIN = 1;     /* 熄灭 LED */
        DelayMs(500);
    }
}