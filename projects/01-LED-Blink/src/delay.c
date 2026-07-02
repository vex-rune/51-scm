/*
 * File:    delay.c
 * Project: 01-LED-Blink
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   基于软件循环的毫秒级阻塞延时（纯 C，无 intrinsics 依赖）
 */

#include <stc89c52rc.h>

/*
 * 内部函数：约 1ms 延时（11.0592 MHz 下校准）
 * 两个空循环嵌套实现约 1ms 延时，不依赖 _nop_()
 */
static void Delay1ms(void)
{
    unsigned char i, j;

    i = 2;
    j = 199;
    do {
        while (--j);
    } while (--i);
}

void DelayMs(unsigned int ms)
{
    while (ms--) {
        Delay1ms();
    }
}