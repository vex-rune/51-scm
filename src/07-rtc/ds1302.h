/*
 * File:    ds1302.h
 * Project: 07-rtc
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   DS1302 实时时钟芯片驱动
 *
 * 硬件: DS1302 接 P37(SCLK) P40(IO) P41(RST)
 */

#ifndef __DS1302_H__
#define __DS1302_H__

#include <stc89c52rc.h>

// 时间结构体 (BCD 编码)
typedef struct {
    unsigned char year;    // 00-99
    unsigned char month;   // 01-12
    unsigned char day;     // 01-31
    unsigned char weekday; // 01-07
    unsigned char hour;    // 00-23
    unsigned char minute;  // 00-59
    unsigned char second;  // 00-59
} DS1302_Time;

// 初始化 DS1302
void DS1302_Init(void);

// 读取时间
void DS1302_ReadTime(DS1302_Time *time);

// 写入时间
void DS1302_WriteTime(const DS1302_Time *time);

// 检查时钟是否在运行 (CH 位)
unsigned char DS1302_IsRunning(void);

#endif /* __DS1302_H__ */
