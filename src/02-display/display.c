/*
 * File:    display.c
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位共阴极数码管显示驱动实现
 *
 * ============================================================
 *  动态扫描原理
 * ============================================================
 *  人眼视觉暂留：图像消失后视觉残留约 1/24s（40ms）。
 *  只要整组数码管刷新周期 ≤ 20ms，肉眼看不到闪烁、画面稳定。
 *
 *  调试只需修改 display.h 中的两个参数：
 *    DISPLAY_DIGITS   数码管位数
 *    DISPLAY_SCAN_US  单管点亮时长（us）
 *
 *  例：8位 × 2000us = 16ms < 20ms → 完全无闪烁
 * ============================================================
 *
 *  时序：先输出新段码，再切换位选；
 *        共阴极+138低有效，位选切换瞬间段码已在缓冲，无鬼影。
 */

#include "display.h"

/* ============================================================
 * 段码表（共阴极：bit=1 亮，bit=0 灭）
 * 排列：b7=dp, b6=g, b5=f, b4=e, b3=d, b2=c, b1=b, b0=a
 * ============================================================ */
const unsigned char SEG_CODE[10] = {
    0x3F,  /* 0: abcdef   */
    0x06,  /* 1: bc       */
    0x5B,  /* 2: abdeg    */
    0x4F,  /* 3: abcdg    */
    0x66,  /* 4: bcfg     */
    0x6D,  /* 5: acdfg    */
    0x7D,  /* 6: acdefg   */
    0x07,  /* 7: abc      */
    0x7F,  /* 8: abcdefg  */
    0x6F,  /* 9: abcdfg   */
};

/* ============================================================
 * 扫描参数（由 display.h 的宏自动计算）
 * ============================================================ */
#define FOSC_HZ       11059200UL                       /* 系统时钟 */
#define T0_PRESCALER  12                               /* 定时器0 预分频 */
#define SCAN_TICKS    ((unsigned int)(FOSC_HZ / 1000000UL * DISPLAY_SCAN_US / T0_PRESCALER))
#define SCAN_RELOAD   (65536UL - SCAN_TICKS)           /* 16位定时器初值 */

/* 显示缓冲区（供外部直接读写） */
unsigned char display_buf[DISPLAY_DIGITS];

/* 当前扫描位索引（0~DISPLAY_DIGITS-1） */
static unsigned char cur_digit = 0;

/* ============================================================
 * 初始化
 * ============================================================ */
void Display_Init(void)
{
    unsigned char i;

    /* 段码初始全灭 */
    P0 = 0x00;

    /* 38译码器初始选通 DIG1 */
    P1 = P1 & 0xC7;

    /* 清空显示缓冲区 */
    for (i = 0; i < DISPLAY_DIGITS; i++) {
        display_buf[i] = 0x00;
    }

    /* 配置定时器0：单管扫描间隔 = DISPLAY_SCAN_US 微秒 */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = (unsigned char)(SCAN_RELOAD >> 8);
    TL0 = (unsigned char)(SCAN_RELOAD & 0xFF);
    ET0 = 1;
    TR0 = 1;
    EA = 1;
}

/* ============================================================
 * 定时器0中断（单管扫描一次）
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    /* 1) 先输出新段码（此时仍选中上一位，但段码先到位） */
    P0 = display_buf[cur_digit];

    /* 2) 切换位选到新位（138低有效，切换瞬间两段码短暂交叠） */
    P1 = (P1 & 0xC7) | ((cur_digit & 0x07) << 3);

    /* 切换到下一位 */
    cur_digit++;
    if (cur_digit >= DISPLAY_DIGITS) {
        cur_digit = 0;
    }

    /* 重装定时初值 */
    TH0 = (unsigned char)(SCAN_RELOAD >> 8);
    TL0 = (unsigned char)(SCAN_RELOAD & 0xFF);
}

/* ============================================================
 * 公开接口
 * ============================================================ */

/**
 * @brief  写入显示缓冲区
 * @param  pos  位号（0~DISPLAY_DIGITS-1）
 * @param  val  段码值
 */
void Display_SetBuf(unsigned char pos, unsigned char val)
{
    if (pos < DISPLAY_DIGITS) {
        display_buf[pos] = val;
    }
}

void Display_UpdateBuf(const char *str)
{
    unsigned char i;
    for (i = 0; i < DISPLAY_DIGITS; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            display_buf[i] = SEG_CODE[str[i] - '0'];
        } else {
            display_buf[i] = 0x00;  // 灭
        }
    }
}
