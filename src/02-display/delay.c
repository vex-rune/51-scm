/*
 * File:    delay.c
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   毫秒级阻塞延时（纯 C，无 intrinsics 依赖）
 */

/* 约 1ms 延时（11.0592 MHz 校准） */
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