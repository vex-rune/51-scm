/*
 * File:    display.c
 * Project: 03-countdown
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位数码管显示驱动实现
 *
 * 动态扫描：定时器0每2ms触发一次中断，
 *           中断中切换当前显示位并输出段码。
 *           8位轮流，刷新率约 62.5 Hz（无闪烁）。
 *
 * 注意：本模块占用定时器0，main.c 使用定时器1做1秒倒计时。
 */

#include "display.h"

/* ============================================================
 * 段码表（共阳极：bit=1 灭，bit=0 亮）
 * 排列：b7=dp, b6=g, b5=f, b4=e, b3=d, b2=c, b1=b, b0=a
 * ============================================================ */
const unsigned char SEG_CODE[16] = {
    0xC0,  /* 0: abcdef   */
    0xF9,  /* 1: bc       */
    0xA4,  /* 2: abdeg    */
    0xB0,  /* 3: abcdg    */
    0x99,  /* 4: bcfg     */
    0x92,  /* 5: acdfg    */
    0x82,  /* 6: acdefg   */
    0xF8,  /* 7: abcf     */
    0x80,  /* 8: abcdefg  */
    0x90,  /* 9: abcdfg   */
    0x88,  /* A: abcefg   */
    0x83,  /* b: cdefg    */
    0xC6,  /* C: adef     */
    0xA1,  /* d: bcdeg    */
    0x86,  /* E: adefg    */
    0x8E,  /* F: aefg     */
};

/* 显示缓冲区 */
unsigned char display_buf[DISPLAY_DIGITS] = {
    SEG_SPACE, SEG_SPACE, SEG_SPACE, SEG_SPACE,
    SEG_SPACE, SEG_SPACE, SEG_SPACE, SEG_SPACE
};

/* 当前扫描位索引 */
static unsigned char cur_digit = 0;

/* ============================================================
 * 74HC595 驱动
 * ============================================================ */

/**
 * @brief  向 74HC595 写入 1 字节（MSB 在前）
 */
static void HC595_SendByte(unsigned char dat)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        HC595_DATA = (dat & 0x80) ? 1 : 0;
        HC595_CLK = 0;
        dat <<= 1;
        HC595_CLK = 1;
    }
}

/**
 * @brief  锁存输出（数据从移位寄存器打入输出锁存器）
 */
static void HC595_Latch(void)
{
    HC595_LATCH = 0;
    HC595_LATCH = 1;
    HC595_LATCH = 0;
}

/* ============================================================
 * 位选控制（共阳极：0=选中，1=关闭）
 * ============================================================ */
static void SelectDigit(unsigned char idx)
{
    P2 = 0xFF;

    switch (idx) {
        case 0: P2 &= ~0x01; break;
        case 1: P2 &= ~0x02; break;
        case 2: P2 &= ~0x04; break;
        case 3: P2 &= ~0x08; break;
        case 4: P2 &= ~0x10; break;
        case 5: P2 &= ~0x20; break;
        case 6: P2 &= ~0x40; break;
        case 7: P2 &= ~0x80; break;
    }
}

/* ============================================================
 * 初始化
 * ============================================================ */
void Display_Init(void)
{
    P2 = 0xFF;

    HC595_DATA = 0;
    HC595_CLK  = 0;
    HC595_LATCH = 0;

    /* 定时器0：2ms 扫描间隔 @ 11.0592MHz */
    /* 计数次数 = 2ms / (12/11059200) = 18432 */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = (65536 - 18432) / 256;
    TL0 = (65536 - 18432) % 256;
    ET0 = 1;
    TR0 = 1;
    EA = 1;
}

/* ============================================================
 * 定时器0中断（2ms 刷新一次）
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    P2 = 0xFF;

    HC595_SendByte(display_buf[cur_digit]);
    HC595_Latch();
    SelectDigit(cur_digit);

    cur_digit++;
    if (cur_digit >= DISPLAY_DIGITS) {
        cur_digit = 0;
    }

    TH0 = (65536 - 18432) / 256;
    TL0 = (65536 - 18432) % 256;
}

/* ============================================================
 * 显示缓冲区操作
 * ============================================================ */
void Display_SetDigit(unsigned char pos, unsigned char value)
{
    if (pos >= DISPLAY_DIGITS) return;

    if (value <= 0x0F) {
        display_buf[pos] = SEG_CODE[value];
    } else if (value == SEG_SPACE) {
        display_buf[pos] = SEG_SPACE;
    } else {
        display_buf[pos] = value;
    }
}

void Display_Clear(void)
{
    unsigned char i;
    for (i = 0; i < DISPLAY_DIGITS; i++) {
        display_buf[i] = SEG_SPACE;
    }
}

void Display_SelfTest(void)
{
    unsigned char i;
    for (i = 0; i < DISPLAY_DIGITS; i++) {
        display_buf[i] = SEG_ALL;
    }
}

/* ============================================================
 * 显示数字
 * ============================================================ */
void Display_ShowUInt(unsigned long num)
{
    signed char pos = 7;

    Display_Clear();

    if (num == 0) {
        display_buf[7] = SEG_CODE[0];
        return;
    }

    while (num > 0 && pos >= 0) {
        display_buf[pos--] = SEG_CODE[num % 10];
        num /= 10;
    }
}
