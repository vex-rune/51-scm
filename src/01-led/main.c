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

/**
 * 引脚说明: 
 * P0.0: LED 接在 P0.0 引脚，低电平点亮（开发板典型接法） 
 */
void main(void)
{
    // 初始化P34为输出模式
    P34 = 1;
    unsigned char led_mask = 0xFE;  // 11111110，最低位点亮
    while (1)
    {
        // 如果都不亮了，就重新从最低位开始点亮
        if (led_mask == 0xFF)
        {
            // 重置为最低位点亮
            led_mask = 0xFE;
        }
        // 循环8个IO口依次点亮流水灯
        for (unsigned char i = 0; i < 8; i++)
        {
            // 点亮当前位
            P0 = led_mask;
            DelayMs(500);
            // 左移一位，高位补1，实现下一个灯亮
            led_mask = (led_mask << 1) | 0x01;
        }
    }
}