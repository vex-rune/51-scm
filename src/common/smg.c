/*
 * File:    smg.c
 * Project: 公共模块 - 7段数码管（SMG = Seven-segment display）
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   共阴极8位数码管动态扫描显示驱动实现
 *
 * ============================================================
 *  动态扫描原理
 * ============================================================
 *  人眼视觉暂留：图像消失后视觉残留约 1/24s（40ms）。
 *  只要整组数码管刷新周期 ≤ 20ms，肉眼看不到闪烁、画面稳定。
 *
 *  调试只需修改 smg.h 中的两个参数：
 *    SMG_DIGITS    数码管位数
 *    SMG_SCAN_US   单管点亮时长（us）
 *
 *  例：8位 × 2000us = 16ms < 20ms → 完全无闪烁
 * ============================================================
 *
 *  时序：先输出新段码，再切换位选；
 *        共阴极+138低有效，位选切换瞬间段码已在缓冲，无鬼影。
 */

#include "smg.h"

/* ============================================================
 * 段码表（共阴极：bit=1 亮，bit=0 灭）
 * 排列：b7=dp, b6=g, b5=f, b4=e, b3=d, b2=c, b1=b, b0=a
 * 索引：'0'..'9' 数字；其它为字母 / 符号（见 smg.h 中 SEG_CODE_INDEX）
 * ============================================================ */
const unsigned char SEG_CODE[SEG_CODE_COUNT] = {
    /* ' ' 空格 */
    0x00,
    /* '!' 感叹号（近似 f 段） */
    0xF9,  /* 1111_1001 -> 仅 e 不亮，不直观，改 0x21: 仅 b 段 */
    /* '-' 减号 */
    0xBF,
    /* '.' 小数点 */
    0x7F,
    /* '0' ~ '9' 数字 */
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
    /* 'A' ~ 'F' 十六进制 */
    0x77,  /* A: abcefg */
    0x7C,  /* b: cdefg  */
    0x39,  /* C: adef   */
    0x5E,  /* d: bcdeg  */
    0x79,  /* E: adefg  */
    0x71,  /* F: aefg   */
    /* 'H' 大写 */
    0x76,  /* H: bcefg  */
    /* 'L' 小写 L */
    0x38,  /* L: adef   */
    /* 'n' 小写 n */
    0x54,  /* n: cdeg   */
    /* 'o' 小写 o */
    0x5C,  /* o: cdeg   */
    /* 'P' 大写 */
    0x73,  /* P: abcefg */
    /* 'U' 大写 */
    0x3E,  /* U: bcdef  */
    /* 'y' 小写 y */
    0x6E,  /* y: bcdfg  */
    /* 'h' 小写 */
    0x74,  /* h: cefg   */
    /* 'r' 小写 */
    0x50,  /* r: eg     */
    /* 't' 小写 */
    0x78,  /* t: defg   */
};

/* ============================================================
 * 扫描参数
 * ============================================================ */
/* 扫描间隔计数：假设调用周期 250us，要达到 SMG_SCAN_US 需要计数 */
#define SMG_SCAN_COUNT  (SMG_SCAN_US / 250)  /* 例如 1000us/250us = 4 */

/* SMG_SEL_A/B/C 在 SMG_SEL_PORT 中的位掩码 */
#define SEL_A_BIT  0x08   /* P1.3 = bit3 */
#define SEL_B_BIT  0x10   /* P1.4 = bit4 */
#define SEL_C_BIT  0x20   /* P1.5 = bit5 */
#define SEL_MASK   (SEL_A_BIT | SEL_B_BIT | SEL_C_BIT)  /* 0x38 */
#define SEL_KEEP   (~SEL_MASK & 0xFF)                   /* 0xC7 */

/* 把 sel(0~7) 写入位选 A/B/C（保留端口其他位） */
#define SEL_WRITE(sel)  do { SMG_SEL_PORT = (SMG_SEL_PORT & SEL_KEEP) | (((sel) & 0x07) << 3); } while (0)

/* 显示缓冲区（供外部直接读写） */
unsigned char smg_buf[SMG_DIGITS];

/* 当前扫描位索引（0~SMG_DIGITS-1） */
static unsigned char cur_digit = 0;

/* 互斥控制标志（由 main.c 设置） */
extern unsigned char g_smg_enabled;

/* ============================================================
 * 内部状态
 * ============================================================ */
static unsigned char scan_counter = 0;

/* ============================================================
 * 初始化
 * ============================================================ */
void Smg_Init(void)
{
    unsigned char i;

    /* 段码初始全灭 */
    SMG_SEG_PORT = 0x00;

    /* 38译码器初始选通 DIG1（cur_digit=0） */
    SEL_WRITE(0);

    /* 清空显示缓冲区 */
    for (i = 0; i < SMG_DIGITS; i++) {
        smg_buf[i] = 0x00;
    }

    scan_counter = 0;
    cur_digit = 0;
}

/* ============================================================
 * 数码管扫描函数（供 timer 模块调用，建议 250us 周期）
 * ============================================================ */
void Smg_Scan(void)
{
    /* 互斥控制 - 只有数码管激活时才扫描 */
    if (!g_smg_enabled) {
        return;
    }

    scan_counter++;
    if (scan_counter < SMG_SCAN_COUNT) {
        return;  /* 未达到扫描间隔，直接返回 */
    }
    scan_counter = 0;

    /* 计算本次要显示的位 */
    unsigned char now = cur_digit;

    /* 1) 关闭段码（消隐，避免位选切换时鬼影） */
    SMG_SEG_PORT = 0x00;

    /* 2) 切换位选到 now */
    SEL_WRITE(now);

    /* 3) 输出新段码（位选已稳定，不会撞到错位） */
    SMG_SEG_PORT = smg_buf[now];

    /* 切到下一位 */
    cur_digit++;
    if (cur_digit >= SMG_DIGITS) {
        cur_digit = 0;
    }
}

/* ============================================================
 * 公开接口
 * ============================================================ */

/**
 * @brief  写入显示缓冲区
 * @param  pos  位号（0~SMG_DIGITS-1）
 * @param  val  段码值
 */
void Smg_SetBuf(unsigned char pos, unsigned char val)
{
    if (pos < SMG_DIGITS) {
        smg_buf[pos] = val;
    }
}

void Smg_UpdateBuf(const char *str)
{
    unsigned char i;
    for (i = 0; i < SMG_DIGITS; i++) {
        unsigned char c = (unsigned char)str[i];
        unsigned char code = SEG_SPACE;  /* 默认识别不出就灭 */

        /* 数字 '0'~'9' */
        if (c >= '0' && c <= '9') {
            code = SEG_CODE[SEG_0 + (c - '0')];
        }
        /* 大写字母 */
        else if (c >= 'A' && c <= 'F') {
            code = SEG_CODE[SEG_A + (c - 'A')];
        }
        else if (c == 'H') code = SEG_CODE[SEG_H];
        else if (c == 'L') code = SEG_CODE[SEG_L];
        else if (c == 'P') code = SEG_CODE[SEG_P];
        else if (c == 'U') code = SEG_CODE[SEG_U];
        /* 小写字母 */
        else if (c == 'n') code = SEG_CODE[SEG_n];
        else if (c == 'o') code = SEG_CODE[SEG_o];
        else if (c == 'y') code = SEG_CODE[SEG_y];
        else if (c == 'h') code = SEG_CODE[SEG_h];
        else if (c == 'r') code = SEG_CODE[SEG_r];
        else if (c == 't') code = SEG_CODE[SEG_t];
        /* 符号 */
        else if (c == '-') code = SEG_CODE[SEG_DASH];
        else if (c == '.') code = SEG_CODE[SEG_DOT];
        else if (c == '!') code = SEG_CODE[SEG_EXCLAIM];
        else if (c == ' ') code = SEG_CODE[SEG_SPACE];

        smg_buf[i] = code;
    }
}

/**
 * @brief  显示无符号整数（高位补空格，0 至少显示 1 个 '0'）
 * @note   例：123 → "     123"，0 → "       0"
 */
void Smg_ShowUInt(unsigned long num)
{
    char buf[SMG_DIGITS + 1];
    signed char i;
    signed char j = 0;
    unsigned long n = num;
    char tmp[SMG_DIGITS];

    if (n == 0) {
        for (i = 0; i < (signed char)(SMG_DIGITS - 1); i++) buf[i] = ' ';
        buf[SMG_DIGITS - 1] = '0';
        buf[SMG_DIGITS] = '\0';
        Smg_UpdateBuf(buf);
        return;
    }

    /* 从低位到高位拆数字 */
    while (n > 0 && j < (signed char)SMG_DIGITS) {
        tmp[j++] = (char)('0' + (n % 10));
        n /= 10;
    }

    /* 高位补空格 */
    for (i = 0; i < (signed char)(SMG_DIGITS - j); i++) {
        buf[i] = ' ';
    }
    /* 数字部分倒序填入 */
    for (i = 0; i < j; i++) {
        buf[SMG_DIGITS - j + i] = tmp[j - 1 - i];
    }
    buf[SMG_DIGITS] = '\0';

    Smg_UpdateBuf(buf);
}
