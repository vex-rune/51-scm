/*
 * File:    ds1302.h
 * Project: 公共模块 - DS1302 实时时钟驱动
 * MCU:     STC89C52RC
 * Brief:   DS1302 RTC 驱动接口
 *
 * 连接:
 *   SCLK -> P37
 *   IO   -> P40
 *   RST  -> P41
 */

#ifndef __DS1302_H__
#define __DS1302_H__

/* 时间结构体 */
typedef struct {
    unsigned char year;     /* 00-99 */
    unsigned char month;    /* 01-12 */
    unsigned char day;      /* 01-31 */
    unsigned char hour;     /* 00-23 */
    unsigned char minute;   /* 00-59 */
    unsigned char second;   /* 00-59 */
    unsigned char weekday;  /* 01-07 */
} DS1302_Time;

/**
 * @brief  初始化 DS1302
 */
void DS1302_Init(void);

/**
 * @brief  读取当前时间
 * @param  time  时间结构体指针
 * @return 0 失败, 1 成功
 */
unsigned char DS1302_ReadTime(DS1302_Time *time);

/**
 * @brief  设置时间
 * @param  time  时间结构体指针
 * @return 0 失败, 1 成功
 */
unsigned char DS1302_SetTime(const DS1302_Time *time);

/**
 * @brief  启动时钟运行
 */
void DS1302_Start(void);

/**
 * @brief  停止时钟运行
 */
void DS1302_Stop(void);

#endif /* __DS1302_H__ */
