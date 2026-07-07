/*
 * File:    main.c
 * Project: 07-i2c
 * MCU:     STC89C52RC
 * Brief:   I2C / EEPROM 测试 - 使用 common 中的 eeprom.c
 */
#include <stc89c52rc.h>
#include <delay.h>
#include "eeprom.h"

/* LED */
#define LED P00

void main(void)
{
    unsigned char count;
    unsigned char i;

    P34 = 1;  /* LED 使能 */
    LED = 1;

    EEPROM_Init();

    /* 读取上次保存的计数值 */
    count = EEPROM_RandomRead(0x00);

    /* 计数值加1，写回 EEPROM */
    count++;
    EEPROM_ByteWrite(0x00, count);

    /* LED 显示计数值（闪烁次数） */
    for (i = 0; i < count; i++) {
        LED = 0;
        DelayMs(200);
        LED = 1;
        DelayMs(200);
    }

    /* 暂停 2 秒后循环 */
    while (1) {
        LED = 1;
        DelayMs(2000);
        LED = 0; DelayMs(200);
        LED = 1; DelayMs(200);
        LED = 0; DelayMs(200);
        LED = 1; DelayMs(200);
    }
}
