/*
 * File:    display.h
 * Project: 02-8digit-display
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8位共阴极数码管显示驱动头文件
 *
 * 硬件连接：
 *   P0.0~P0.7  -> 段码输出（a~dp）
 *   P1.3       -> 38译码器 A
 *   P1.4       -> 38译码器 B
 *   P1.5       -> 38译码器 C
 *   38译码器 Y0~Y7 -> 位选 DIG1~DIG8
 *
 * 使用方式：
 *   1. Display_Init() 初始化
 *   2. 直接操作 display_buf[] 写入显示内容
 *   3. 定时器0中断（2ms）自动从缓冲区读取并扫描显示
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stc89c52rc.h>

/* ============================================================
 * 可调参数（调试时只需修改这两行）
 * ============================================================ */
#define DISPLAY_DIGITS    8        /* 数码管位数 */
#define DISPLAY_SCAN_US   400     /* 单管点亮时长（微秒） */

/* 推荐范围：
 *   DISPLAY_DIGITS  = 1~16      （实际硬件位数）
 *   DISPLAY_SCAN_US = 1000~2000 （1~2ms；总周期 = DIGITS × SCAN_US）
 *
 * 闪烁判据：
 *   总周期 ≤ 20000us  → 无闪烁
 *   总周期 > 30000us  → 明显频闪
 */

/* ============================================================
 * 段码表（共阴极：bit=1 亮，bit=0 灭）
 * 排列：b7=dp, b6=g, b5=f, b4=e, b3=d, b2=c, b1=b, b0=a
 * ============================================================ */
extern const unsigned char SEG_CODE[10];  /* 0~9 */

/* 显示缓冲区（供外部直接读写） */
extern unsigned char display_buf[DISPLAY_DIGITS];

/* ============================================================
 * 函数声明
 * ============================================================ */

/**
 * @brief  初始化显示驱动
 * @note   占用定时器0，配置为 2ms 中断，自动扫描显示
 */
void Display_Init(void);

/**
 * @brief  定时器0中断服务程序（2ms 刷新一次，动态扫描）
 * @note   用户不应直接调用，由定时器中断自动触发
 */
void Timer0_ISR(void) __interrupt(1);

/**
 * @brief  写入显示缓冲区
 * @param  pos  位号（0~7，对应 DIG1~DIG8）
 * @param  val  段码值（使用 SEG_CODE[n] 或 0x00 灭）
 */
void Display_SetBuf(unsigned char pos, unsigned char val);

/**
 * @brief  全量更新显示缓冲区
 * @param  str  8位字符串（'0'-'9'显示数字，其他字符灭）
 */
void Display_UpdateBuf(const char *str);

#endif /* __DISPLAY_H__ */
