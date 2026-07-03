/*
 * File:    display.h
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位数码管显示驱动头文件
 *
 * 硬件连接：
 *   P1.0 → 74HC595 DATA (DS)
 *   P1.1 → 74HC595 CLK  (SH_CP)
 *   P1.2 → 74HC595 LATCH(ST_CP)
 *   P2.0~P2.7 → 位选 PNP 三极管基极 (Digit 0~7)
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stc89c52rc.h>

/* ============================================================
 * 硬件接口定义
 * ============================================================ */

/* 74HC595 控制引脚（P1.0 / P1.1 / P1.2） */
#define HC595_DATA   P1_0
#define HC595_CLK    P1_1
#define HC595_LATCH  P1_2

/* 位选引脚（低电平选中数码管，共阳极） */
#define DIGIT_PIN0   P2_0
#define DIGIT_PIN1   P2_1
#define DIGIT_PIN2   P2_2
#define DIGIT_PIN3   P2_3
#define DIGIT_PIN4   P2_4
#define DIGIT_PIN5   P2_5
#define DIGIT_PIN6   P2_6
#define DIGIT_PIN7   P2_7

/* 数码管位数 */
#define DISPLAY_DIGITS  8

/* ============================================================
 * 段码表（共阳极：1=灭，0=亮）
 * bit:  dp  g   f   e   d   c   b   a
 * 位置:  7   6   5   4   3   2   1   0
 * ============================================================ */
extern const unsigned char SEG_CODE[16];  /* 0~9, A~F */

/* 特殊字符 */
#define SEG_SPACE  0xFF   /* 全灭 */
#define SEG_DASH   0xBF   /* '-' (g段亮) */
#define SEG_DOT    0x7F   /* '.' (dp段亮) */
#define SEG_ALL    0x00   /* 全亮（自检用） */

/* ============================================================
 * 显示缓冲区
 * ============================================================ */
extern unsigned char display_buf[DISPLAY_DIGITS];

/* ============================================================
 * 函数声明
 * ============================================================ */

/**
 * @brief  初始化显示驱动（IO 方向、定时器0）
 */
void Display_Init(void);

/**
 * @brief  刷新显示缓冲区，将数字填入指定位置
 * @param  pos    位置（0=最低位，7=最高位）
 * @param  value  0~15 对应 SEG_CODE，255=空格
 */
void Display_SetDigit(unsigned char pos, unsigned char value);

/**
 * @brief  显示有符号整数（支持负数，最高位显示'-'）
 * @param  num  要显示的数字（-9999999 ~ 99999999）
 */
void Display_ShowInt(long num);

/**
 * @brief  显示无符号整数
 * @param  num  要显示的数字（0 ~ 99999999）
 */
void Display_ShowUInt(unsigned long num);

/**
 * @brief  清除所有显示位（显示空格）
 */
void Display_Clear(void);

/**
 * @brief  显示自检（全亮 1 秒后全灭）
 */
void Display_SelfTest(void);

/**
 * @brief  定时器0中断服务程序（2ms 刷新一次，动态扫描）
 * @note   用户不应直接调用，由定时器中断自动触发
 */
void Timer0_ISR(void) __interrupt(1);

#endif /* __DISPLAY_H__ */