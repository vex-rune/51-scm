/*
 * File:    smg.h
 * Project: 公共模块 - 7段数码管（SMG = Seven-segment display）
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   共阴极8位数码管动态扫描显示驱动头文件
 *
 * 硬件连接：
 *   SMG_SEG_PORT   -> 段码输出（a~dp）
 *   SMG_SEL_A/B/C  -> 38译码器 A/B/C
 *   38译码器 Y0~Y7 -> 位选 DIG1~DIG8
 *
 * 使用方式：
 *   1. Smg_Init() 初始化
 *   2. 直接操作 smg_buf[] 写入显示内容
 *   3. 定时器0中断自动从缓冲区读取并扫描显示
 */

#ifndef __SMG_H__
#define __SMG_H__

#include <stc89c52rc.h>

/* ============================================================
 * 硬件引脚配置（移植时只需修改这里）
 *   本头文件使用项目自定义 SFR 位名（P13/P14/P15），
 *   如改用 Keil 风格的 P1_3 需调整。
 * ============================================================ */
#define SMG_SEG_PORT   P0      /* 段码输出端口（P0.0~P0.7 -> a~dp） */
#define SMG_SEL_PORT   P1      /* 位选信号所在端口（38译码器A/B/C） */
#define SMG_SEL_A      P13     /* 38译码器 A  -> P1.3 */
#define SMG_SEL_B      P14     /* 38译码器 B  -> P1.4 */
#define SMG_SEL_C      P15     /* 38译码器 C  -> P1.5 */

/* ============================================================
 * 可调参数（调试时只需修改这两行）
 * ============================================================ */
#define SMG_DIGITS    8        /* 数码管位数 */
#define SMG_SCAN_US   2000     /* 单管点亮时长（微秒） */

/* 推荐范围：
 *   SMG_DIGITS  = 1~16      （实际硬件位数）
 *   SMG_SCAN_US = 1000~2000 （1~2ms；总周期 = DIGITS × SCAN_US）
 *
 * 闪烁判据：
 *   总周期 ≤ 20000us  → 无闪烁
 *   总周期 > 30000us  → 明显频闪
 */

/* ============================================================
 * 段码表（共阴极：bit=1 亮，bit=0 灭）
 * 排列：b7=dp, b6=g, b5=f, b4=e, b3=d, b2=c, b1=b, b0=a
 *
 * 通过 SEG_CODE_INDEX 枚举访问：
 *   SEG_CODE[SEG_0]      → 数字 0 的段码
 *   SEG_CODE[SEG_DASH]   → '-' 的段码
 *   SEG_CODE[SEG_DOT]    → '.' 的段码
 * ============================================================ */
typedef enum {
    SEG_SPACE = 0,   /* ' '  全灭 */
    SEG_EXCLAIM,     /* '!'  近似感叹号 */
    SEG_DASH,        /* '-'  中横线（g段） */
    SEG_DOT,         /* '.'  小数点（dp段） */
    SEG_0,           /* '0'  数字 0 */
    SEG_1, SEG_2, SEG_3, SEG_4, SEG_5, SEG_6, SEG_7, SEG_8, SEG_9,
    SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F,   /* A~F */
    SEG_H,           /* 'H' */
    SEG_L,           /* 'L' */
    SEG_n,           /* 'n' */
    SEG_o,           /* 'o' */
    SEG_P,           /* 'P' */
    SEG_U,           /* 'U' */
    SEG_y,           /* 'y' */
    SEG_h,           /* 'h' */
    SEG_r,           /* 'r' */
    SEG_t,           /* 't' */
    SEG_CODE_COUNT   /* 段码总数（数组大小） */
} SEG_CODE_INDEX;

extern const unsigned char SEG_CODE[SEG_CODE_COUNT];

/* 显示缓冲区（供外部直接读写） */
extern unsigned char smg_buf[SMG_DIGITS];

/* ============================================================
 * 函数声明
 * ============================================================ */

/**
 * @brief  初始化数码管显示驱动
 * @note   占用定时器0，配置为 SMG_SCAN_US 中断，自动扫描显示
 */
void Smg_Init(void);

/**
 * @brief  定时器0中断服务程序（动态扫描）
 * @note   用户不应直接调用，由定时器中断自动触发
 */
void Smg_Timer0_ISR(void) __interrupt(1);

/**
 * @brief  写入显示缓冲区
 * @param  pos  位号（0~SMG_DIGITS-1）
 * @param  val  段码值（使用 SEG_CODE[n] 或 0x00 灭）
 */
void Smg_SetBuf(unsigned char pos, unsigned char val);

/**
 * @brief  全量更新显示缓冲区
 * @param  str  字符串（'0'-'9'显示数字，其他字符灭）
 */
void Smg_UpdateBuf(const char *str);

/**
 * @brief  显示无符号整数（高位补空格）
 * @param  num  要显示的数字（0 ~ 99999999）
 */
void Smg_ShowUInt(unsigned long num);

#endif /* __SMG_H__ */
