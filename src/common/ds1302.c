/*
 * File:    ds1302.c
 * Project: 公共模块 - DS1302 实时时钟驱动实现
 * MCU:     STC89C52RC
 * Brief:   DS1302 RTC 驱动
 *
 * 连接:
 *   SCLK -> P37
 *   IO   -> P40
 *   RST  -> P41
 */

#include <stc89c52rc.h>
#include "ds1302.h"

/* 引脚定义 */
#define DS1302_SCLK  P37
#define DS1302_IO    P40
#define DS1302_RST   P41

/* DS1302 命令 */
#define DS1302_SECOND       0x80
#define DS1302_MINUTE       0x82
#define DS1302_HOUR         0x84
#define DS1302_DATE         0x86
#define DS1302_MONTH        0x88
#define DS1302_DAY          0x8A
#define DS1302_YEAR         0x8C
#define DS1302_CONTROL      0x8E
#define DS1302_CHARGER      0x90
#define DS1302_CLKBURST     0xBE
#define DS1302_RAMBURST     0xFE

/* 延时 */
#define DS1302_DELAY()  __asm nop __endasm

/**
 * @brief  向 DS1302 写 1 字节
 */
static void DS1302_WriteByte(unsigned char dat)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DS1302_SCLK = 0;
        DS1302_IO = dat & 0x01;
        DS1302_DELAY();
        DS1302_SCLK = 1;
        DS1302_DELAY();
        dat >>= 1;
    }
}

/**
 * @brief  从 DS1302 读 1 字节
 */
static unsigned char DS1302_ReadByte(void)
{
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {
        DS1302_SCLK = 0;
        DS1302_DELAY();
        dat >>= 1;
        if (DS1302_IO) dat |= 0x80;
        DS1302_SCLK = 1;
        DS1302_DELAY();
    }
    return dat;
}

/**
 * @brief  向指定寄存器写数据
 */
static void DS1302_Write(unsigned char addr, unsigned char dat)
{
    DS1302_RST = 0;
    DS1302_SCLK = 0;
    DS1302_RST = 1;
    DS1302_WriteByte(addr);
    DS1302_WriteByte(dat);
    DS1302_SCLK = 0;
    DS1302_RST = 0;
}

/**
 * @brief  从指定寄存器读数据
 */
static unsigned char DS1302_Read(unsigned char addr)
{
    unsigned char dat;
    DS1302_RST = 0;
    DS1302_SCLK = 0;
    DS1302_RST = 1;
    DS1302_WriteByte(addr | 0x01);  /* 读命令 */
    dat = DS1302_ReadByte();
    DS1302_SCLK = 0;
    DS1302_RST = 0;
    return dat;
}

/**
 * @brief  BCD 转十进制
 */
static unsigned char BCD2Dec(unsigned char bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/**
 * @brief  十进制转 BCD
 */
static unsigned char Dec2BCD(unsigned char dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

/* ============================================================
 * API 实现
 * ============================================================ */

void DS1302_Init(void)
{
    DS1302_RST = 0;
    DS1302_SCLK = 0;
    DS1302_IO = 0;
    
    /* 关闭写保护 */
    DS1302_Write(DS1302_CONTROL, 0x00);
}

unsigned char DS1302_ReadTime(DS1302_Time *time)
{
    unsigned char tmp;
    
    tmp = DS1302_Read(DS1302_SECOND);
    time->second = BCD2Dec(tmp & 0x7F);  /* 去掉 CH 位 */
    
    tmp = DS1302_Read(DS1302_MINUTE);
    time->minute = BCD2Dec(tmp);
    
    tmp = DS1302_Read(DS1302_HOUR);
    time->hour = BCD2Dec(tmp & 0x3F);  /* 24 小时制 */
    
    tmp = DS1302_Read(DS1302_DATE);
    time->day = BCD2Dec(tmp);
    
    tmp = DS1302_Read(DS1302_MONTH);
    time->month = BCD2Dec(tmp);
    
    tmp = DS1302_Read(DS1302_DAY);
    time->weekday = BCD2Dec(tmp);
    
    tmp = DS1302_Read(DS1302_YEAR);
    time->year = BCD2Dec(tmp);
    
    return 1;
}

unsigned char DS1302_SetTime(const DS1302_Time *time)
{
    /* 关闭写保护 */
    DS1302_Write(DS1302_CONTROL, 0x00);
    
    DS1302_Write(DS1302_YEAR, Dec2BCD(time->year));
    DS1302_Write(DS1302_MONTH, Dec2BCD(time->month));
    DS1302_Write(DS1302_DATE, Dec2BCD(time->day));
    DS1302_Write(DS1302_HOUR, Dec2BCD(time->hour));
    DS1302_Write(DS1302_MINUTE, Dec2BCD(time->minute));
    DS1302_Write(DS1302_SECOND, Dec2BCD(time->second));
    DS1302_Write(DS1302_DAY, Dec2BCD(time->weekday));
    
    /* 开启写保护 */
    DS1302_Write(DS1302_CONTROL, 0x80);
    
    return 1;
}

void DS1302_Start(void)
{
    unsigned char tmp = DS1302_Read(DS1302_SECOND);
    DS1302_Write(DS1302_SECOND, tmp & 0x7F);  /* 清除 CH 位 */
}

void DS1302_Stop(void)
{
    unsigned char tmp = DS1302_Read(DS1302_SECOND);
    DS1302_Write(DS1302_SECOND, tmp | 0x80);  /* 设置 CH 位 */
}
