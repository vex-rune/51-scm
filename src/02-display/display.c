/*
 * File:    display.c
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位数码管显示驱动实现
 *
 * 动态扫描：定时器0每2ms触发一次中断，
 *           中断中切换当前显示位并输出段码。
 *           8位轮流，刷新率约 62.5 Hz（无闪烁）。
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
        HC595_DATA = (dat & 0x80) ? 1 : 0;  /* 发送最高位 */
        HC595_CLK = 0;                       /* 上升沿移位 */
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
    HC595_LATCH = 1;  /* 上升沿锁存 */
    HC595_LATCH = 0;
}

/* ============================================================
 * 位选控制（共阳极：0=选中，1=关闭）
 * ============================================================ */
static void SelectDigit(unsigned char idx)
{
    P2 = 0xFF;  /* 先全部关闭（PNP 截止） */

    switch (idx) {
        case 0: P2 &= ~0x01; break;  /* P2.0 = 0 */
        case 1: P2 &= ~0x02; break;  /* P2.1 = 0 */
        case 2: P2 &= ~0x04; break;  /* P2.2 = 0 */
        case 3: P2 &= ~0x08; break;  /* P2.3 = 0 */
        case 4: P2 &= ~0x10; break;  /* P2.4 = 0 */
        case 5: P2 &= ~0x20; break;  /* P2.5 = 0 */
        case 6: P2 &= ~0x40; break;  /* P2.6 = 0 */
        case 7: P2 &= ~0x80; break;  /* P2.7 = 0 */
    }
}

/* ============================================================
 * 初始化
 * ============================================================ */
void Display_Init(void)
{
    /* P1.0~P1.2 准双向模式（SDCC 默认，74HC595 驱动足够） */
    /* P2 准双向模式（默认，可直接驱动 PNP 三极管基极） */
    P2 = 0xFF;  /* 初始全高=全部关闭（共阳极，PNP 截止） */

    /* 74HC595 初始状态 */
    HC595_DATA = 0;
    HC595_CLK  = 0;
    HC595_LATCH = 0;

    /* 定时器0初始化（2ms @ 11.0592MHz） */
    /* 12分频，计数 18432 次 = 2ms */
    TMOD &= 0xF0;
    TMOD |= 0x01;   /* 模式1：16位定时器 */
    TH0 = (65536 - 18432) / 256;
    TL0 = (65536 - 18432) % 256;
    ET0 = 1;        /* 开定时器0中断 */
    TR0 = 1;        /* 启动定时器0 */
    EA = 1;         /* 开总中断 */
}

/* ============================================================
 * 定时器0中断（2ms 刷新一次）
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    /* 先关闭当前位 */
    P2 = 0xFF;

    /* 取出段码并送到 74HC595 */
    HC595_SendByte(display_buf[cur_digit]);
    HC595_Latch();

    /* 选中下一位 */
    SelectDigit(cur_digit);

    /* 指向下一位 */
    cur_digit++;
    if (cur_digit >= DISPLAY_DIGITS) {
        cur_digit = 0;
    }

    /* 重装定时初值 */
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
        display_buf[pos] = value;  /* 直接写入段码 */
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
    /* 延时 1 秒后清除 */
    /* (延时函数由外部调用 DelayMs) */
}

/* ============================================================
 * 显示数字（简化版：右对齐显示）
 * ============================================================ */
void Display_ShowInt(long num)
{
    unsigned char i;
    unsigned char digits[DISPLAY_DIGITS];
    unsigned char count = 0;
    unsigned char negative = 0;

    Display_Clear();

    if (num < 0) {
        negative = 1;
        num = -num;
    }

    if (num == 0) {
        display_buf[7] = SEG_CODE[0];
        return;
    }

    while (num > 0 && count < DISPLAY_DIGITS) {
        digits[count++] = num % 10;
        num /= 10;
    }

    if (negative && count < DISPLAY_DIGITS) {
        display_buf[7 - count] = SEG_DASH;
    }

    for (i = 0; i < count; i++) {
        display_buf[7 - i] = SEG_CODE[digits[count - 1 - i]];
    }
}

void Display_ShowUInt(unsigned long num)
{
    Display_Clear();

    if (num == 0) {
        display_buf[7] = SEG_CODE[0];
        return;
    }

    /* 从最低位开始填入缓冲区（高位留空格） */
    signed char pos = 7;
    while (num > 0 && pos >= 0) {
        display_buf[pos--] = SEG_CODE[num % 10];
        num /= 10;
    }
}