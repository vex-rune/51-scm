/*
 * File:    main.c
 * Project: 11-DS1302
 * MCU:     STC89C52RC
 * Brief:   DS1302 实时时钟 + OLED 显示
 *
 * 连接:
 *   DS1302-SCLK -> P37
 *   DS1302-IO   -> P40
 *   DS1302-RST  -> P41
 */
#include <stc89c52rc.h>
#include <delay.h>
#include <oled.h>
#include <ds1302.h>

/**
 * @brief  显示两位数字
 */
static void Show2Digit(unsigned char page, unsigned char col, unsigned char val)
{
    OLED_ShowAsciiAt(page, col, '0' + (val / 10));
    OLED_ShowAsciiAt(page, col + 8, '0' + (val % 10));
}

int main(void)
{
    DS1302_Time time;
    unsigned char init = 0;
    
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, "DS1302 RTC");
    
    DS1302_Init();
    
    /* 初始化时间: 2026-07-09 周四 15:30:00 */
    if (!init) {
        time.year = 26;
        time.month = 7;
        time.day = 9;
        time.weekday = 4;  /* 周四 */
        time.hour = 15;
        time.minute = 30;
        time.second = 0;
        DS1302_SetTime(&time);
        init = 1;
    }
    
    DS1302_Start();  /* 确保时钟运行 */
    
    while (1) {
        if (DS1302_ReadTime(&time)) {
            /* 显示日期: 26-07-09 */
            OLED_ShowString(2, 0, "Date:");
            Show2Digit(2, 6*8, time.year);
            OLED_ShowAsciiAt(2, 8*8, '-');
            Show2Digit(2, 9*8, time.month);
            OLED_ShowAsciiAt(2, 11*8, '-');
            Show2Digit(2, 12*8, time.day);
            
            /* 显示星期 */
            OLED_ShowString(2, 15*8, "W");
            OLED_ShowAsciiAt(2, 16*8, '0' + time.weekday);
            
            /* 显示时间: 15:30:00 */
            OLED_ShowString(4, 0, "Time:");
            Show2Digit(4, 6*8, time.hour);
            OLED_ShowAsciiAt(4, 8*8, ':');
            Show2Digit(4, 9*8, time.minute);
            OLED_ShowAsciiAt(4, 11*8, ':');
            Show2Digit(4, 12*8, time.second);
        } else {
            OLED_ShowString(2, 0, "Read Error!");
        }
        
        DelayMs(1000);  /* 每秒更新 */
    }
}
