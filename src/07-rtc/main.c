/*
 * File:    main.c
 * Project: 07-rtc
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   DS1302 实时时钟读取，并通过串口输出时间
 *
 * 硬件: DS1302 接 P37(SCLK) P40(IO) P41(RST)
 *       USB 转 TTL 接 P3.0(RXD)/P3.1(TXD)
 *
 * 串口助手: 9600 / 8N1
 */

#include <stc89c52rc.h>
#include "delay.h"
#include "uart.h"
#include "ds1302.h"

static const char *WEEKDAY_NAMES[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void main(void)
{
    DS1302_Time time;
    char buf[64];

    Uart_Init();
    DS1302_Init();

    Uart_SendString("=== 07-rtc ready ===\r\n");

    // 如果时钟未运行，设置初始时间 (2024-01-01 00:00:00)
    if (!DS1302_IsRunning()) {
        time.year   = 24;
        time.month  = 1;
        time.day    = 1;
        time.weekday = 1;
        time.hour   = 0;
        time.minute = 0;
        time.second = 0;
        DS1302_WriteTime(&time);
        Uart_SendString("RTC initialized to 2024-01-01 00:00:00\r\n");
    }

    while (1) {
        DS1302_ReadTime(&time);

        // 格式化: 2024-01-01 12:34:56 Mon
        buf[0] = '2';
        buf[1] = '0';
        buf[2] = (time.year / 10) + '0';
        buf[3] = (time.year % 10) + '0';
        buf[4] = '-';
        buf[5] = (time.month / 10) + '0';
        buf[6] = (time.month % 10) + '0';
        buf[7] = '-';
        buf[8] = (time.day / 10) + '0';
        buf[9] = (time.day % 10) + '0';
        buf[10] = ' ';
        buf[11] = (time.hour / 10) + '0';
        buf[12] = (time.hour % 10) + '0';
        buf[13] = ':';
        buf[14] = (time.minute / 10) + '0';
        buf[15] = (time.minute % 10) + '0';
        buf[16] = ':';
        buf[17] = (time.second / 10) + '0';
        buf[18] = (time.second % 10) + '0';
        buf[19] = ' ';
        buf[20] = '\0';

        Uart_SendString(buf);
        Uart_SendString(WEEKDAY_NAMES[time.weekday - 1]);
        Uart_SendString("\r\n");

        DelayMs(1000);
    }
}
