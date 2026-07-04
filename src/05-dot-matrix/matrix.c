/*
 * File:    matrix.c
 * Project: 05-dot-matrix
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 * Brief:   8x8 LED 点阵显示驱动实现
 *
 * 扫描原理:
 *   - 当前显示图案存于 frame_buf[8]
 *   - T0 每 1ms 中断一次, 切换到下一行
 *   - 行选通过 595 串行移位实现 (点亮行 = 0, 灭行 = 1)
 *   - 列数据直接送 P3 (共阴极: 0=亮, 1=灭)
 *
 * 74HC595 引脚分配 (DIP-16):
 *   QA (pin 15) -> D0 (第 0 行)
 *   QB (pin  1) -> D1 (第 1 行)
 *   ...
 *   QH (pin  7) -> D7 (第 7 行)
 *   QH' (pin 9)  未使用 (无级联)
 */

#include "matrix.h"

/* ============================================================
 * 当前显示的图案 (由 Matrix_Show 写入)
 * ============================================================ */
static unsigned char frame_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 当前扫描行 (0~7) */
static unsigned char cur_row = 0;

/* ============================================================
 * 扫描一行
 *   行选: 1 字节送 595, Q0~Q7 -> D0~D7
 *         bit=0 表示该行点亮 (阳极高)
 *         bit=1 表示该行熄灭
 *   列选: 直接写 P3, bit=0 表示该列点亮
 *         (共阴极: 行高 + 列低 = 该点亮)
 * ============================================================ */
static void ScanRow(unsigned char row)
{
    unsigned char i;
    unsigned char row_mask;
    unsigned char col_data;

    /* 假设 row=2 (扫描第 3 行)
     *
     * row_mask = ~(0x01 << 2) = ~0b00000100 = 0b11111011
     *                    ↑       ↑
     *                  bit2=0  其余全 1
     *
     * 这个字节送到 595 后:
     *   QA (D0) = 1 -> 第 0 行不亮
     *   QB (D1) = 1 -> 第 1 行不亮
     *   QC (D2) = 0 -> 第 2 行亮 (阳极高)  <- 当前扫描行
     *   QD (D3) = 1 -> 第 3 行不亮
     *   ... 其余 1
     *
     * 因为一帧只有 1 个 bit=0, 所以只有 1 行被点亮
     */
    row_mask = (unsigned char)~(0x01 << row);

    /* 假设 frame_buf[2] = 0b01100000 (第 2 行的图案)
     *
     * col_data = ~0b01100000 = 0b10011111
     *
     * 写到 P3 后:
     *   P3.0 = 1 -> 第 0 列不亮
     *   P3.1 = 1 -> 第 1 列不亮
     *   P3.2 = 0 -> 第 2 列亮 (阴极低)
     *   P3.3 = 0 -> 第 3 列亮
     *   P3.4 = 1 -> 第 4 列不亮
     *   ... 其余略
     *
     * 共阴极逻辑: 行高 + 列低 = 该 LED 亮
     * 当前第 2 行是高, 第 2,3 列是低 -> 第 2 行的第 2,3 个 LED 亮
     */
    col_data = (unsigned char)~frame_buf[row];

    /* 先把列数据放上 P3, 等 595 锁存时一起生效 */
    P3 = col_data;

    /* 串行发送 row_mask 到 595, MSB 先发
     *
     * 时序: SCK 每次上升沿把 SER 的当前值推进移位寄存器
     * 发送顺序: bit7 -> bit6 -> ... -> bit0
     *
     * 例 row_mask = 0b11111011:
     *   第 1 次: SER=1,  SCK↑  -> 寄存器: 1....... (bit7 入)
     *   第 2 次: SER=1,  SCK↑  -> 寄存器: 11...... (bit6 入)
     *   第 3 次: SER=1,  SCK↑  -> 寄存器: 111..... (bit5 入)
     *   第 4 次: SER=1,  SCK↑  -> 寄存器: 1111.... (bit4 入)
     *   第 5 次: SER=1,  SCK↑  -> 寄存器: 11111... (bit3 入)
     *   第 6 次: SER=0,  SCK↑  -> 寄存器: 111110.. (bit2=0 入) <- 当前行
     *   第 7 次: SER=1,  SCK↑  -> 寄存器: 1111101. (bit1 入)
     *   第 8 次: SER=1,  SCK↑  -> 寄存器: 11111011 (bit0 入) 完成
     */
    for (i = 0; i < 8; i++) {
        HC595_DATA = (row_mask & 0x80) ? 1 : 0;
        HC595_CLK = 0;
        row_mask <<= 1;
        HC595_CLK = 1;
    }

    /* 锁存: 上升沿把移位寄存器数据送输出 */
    HC595_LATCH = 0;
    HC595_LATCH = 1;
    HC595_LATCH = 0;
}

/* ============================================================
 * 初始化
 * ============================================================ */
void Matrix_Init(void)
{
    unsigned char i;

    /* 清空图案 */
    for (i = 0; i < 8; i++) {
        frame_buf[i] = 0;
    }
    cur_row = 0;

    /* 595 控制引脚初始电平 */
    HC595_DATA = 0;
    HC595_CLK  = 0;
    HC595_LATCH = 0;

    /* 全部列拉高 (全部灭) */
    P3 = 0xFF;

    /* T0: 模式1, 1ms @ 11.0592MHz */
    /* 计数次数 = 1ms / (12/11059200) = 9216 */
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = (65536 - 9216) / 256;
    TL0 = (65536 - 9216) % 256;
    ET0 = 1;
    TR0 = 1;
    EA  = 1;
}

/* ============================================================
 * 公共 API
 * ============================================================ */
void Matrix_Show(const unsigned char pic[8])
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        frame_buf[i] = pic[i];
    }
}

void Matrix_Clear(void)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        frame_buf[i] = 0;
    }
}

void Matrix_SelfTest(void)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        frame_buf[i] = 0xFF;
    }
}

/* ============================================================
 * 播放动画 (阻塞)
 * ============================================================ */
void Matrix_PlayAnimation(const unsigned char *frames,
                          unsigned char frame_count,
                          unsigned int  interval_ms)
{
    unsigned char i;
    unsigned int  j;
    unsigned char k;

    if (frames == 0 || frame_count == 0) return;

    for (k = 0; k < frame_count; k++) {
        Matrix_Show(&frames[k * 8]);

        /* 软件延时 (后台扫描照常进行) */
        for (j = 0; j < interval_ms; j++) {
            for (i = 0; i < 120; i++) {
                ;
            }
        }
    }
}

/* ============================================================
 * T0 中断服务程序: 每 1ms 扫描一行
 *
 * 触发条件 (前置条件, 缺一不可):
 *   1. EA  = 1   (总中断开关, 在 Matrix_Init 里开)
 *   2. ET0 = 1   (T0 局部中断开关, 在 Matrix_Init 里开)
 *   3. TR0 = 1   (T0 启动, 计数器开始 +1, 在 Matrix_Init 里开)
 *   4. T0 模式1 (16位定时器, TMOD |= 0x01)
 *   5. 计数器从初值加到 65536 溢出 (本程序设 57536 = 65536 - 9216)
 *
 * 触发时机:
 *   - 11.0592MHz 晶振, 12T 模式, 机器周期 = 12/11059200 ≈ 1.085μs
 *   - T0 每次机器周期 +1, 从 57536 加到 65536 需要 9216 次
 *   - 11.0592MHz / 12 / 9216 ≈ 1000Hz, 周期 = 1ms
 *
 * 调用流程 (硬件自动, 不是 C 代码调用):
 *   计数器溢出 -> 硬件置 TF0=1
 *              -> CPU 检查 EA=1 && ET0=1 (满足)
 *              -> 硬件把 PC 压栈
 *              -> CPU 跳转到 0x000B (T0 中断向量)
 *              -> 0x000B 处的 LJMP 指令 -> 跳到这里
 *
 * 注意:
 *   - 不能在 main.c 里手动调用 Timer0_ISR()
 *   - 函数末尾的 RETI 指令会硬件自动撤销中断优先级锁
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    /* 1. 扫描当前行 (根据 cur_row 算出 row_mask, 送到 595, 锁存) */
    ScanRow(cur_row);

    /* 2. cur_row 切换到下一行 (0->1->2->...->7->0 循环) */
    cur_row++;
    if (cur_row >= MATRIX_ROWS) {
        cur_row = 0;
    }

    /* 3. 重装定时初值 (模式1 不会自动重装, 必须手动)
     *    不重装的话, 下次会从 65536 累加, 周期变成 65536 × 1.085μs ≈ 71ms */
    TH0 = (65536 - 9216) / 256;
    TL0 = (65536 - 9216) % 256;
}

/* ============================================================
 * 滚动显示模块
 *   把字模数组 (N 字符 × 8 字节) 拼成超宽图
 *   每次显示 8 列, offset 每次 +1, 字符从右往左滑
 * ============================================================ */

/* 简易软件延时 (滚动用, 内部用) */
static void ScrollDelayMs(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++) {
            ;
        }
    }
}

/* 滚动缓冲区 (8 字节) */
static unsigned char scroll_buf[8];

/* ScrollInit: 保留接口便于未来扩展 */
void Matrix_ScrollInit(void)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        scroll_buf[i] = 0;
    }
}

/* 核心: 显示一帧, 偏移量为 offset (整张超宽图里的列号)
 *   (保留接口便于未来扩展, 当前未使用)
 */
static void Scroll_ShowFrame(unsigned int offset, unsigned int speed_ms)
{
    (void)offset;
    (void)speed_ms;
}

/* 滚动显示: 把字模数组拼成超宽图, 每次显示其中 8 列
 *   每 step+1, 整个超宽图向左滑 1 列 (屏幕看到的图案也左移 1 列)
 *   屏幕上某 1 列可能来自 2 个相邻字符的边界
 *
 *   font:       字模数组, char_count × 8 字节 (在 ROM)
 *               格式: [c0r0, c0r1, ..., c0r7, c1r0, ...]
 *   char_count: 字符总数
 *   speed_ms:   每步间隔 (ms)
 */
void Matrix_PlayScroll(const unsigned char __code *font,
                       unsigned char char_count,
                       unsigned int speed_ms)
{
    /* 局部变量声明 */
    unsigned char i;             /* 屏幕列索引 0~7, 循环变量 */
    unsigned int  step;          /* 滚动总步数, 每步 = 1 列 */

    /* 参数合法性检查:
     *   font == 0      -> 指针为空, 没法读字模
     *   char_count==0  -> 0 字符, 滚个啥
     *   都不处理, 直接返回 */
    if (font == 0 || char_count == 0) return;

    /* 总步数推导:
     *   字符要"完全在屏幕右边外" -> 字符要"完全在屏幕左边外"
     *
     *   起始:                    结束:
     *   ┌────────┐               ┌────────┐
     *   │        │  --->         │        │
     *   └────────┘               └────────┘
     *        ↑                       ↑
     *   字符看不见             字符看不见
     *
     *   要滑过的距离 = 字符总宽(char_count * 8) + 屏幕宽(8)
     *
     *   验证 (17 字符):
     *     step=0      -> 屏幕空, 字符完全在右外
     *     step=8      -> 字符最左端刚进入右沿
     *     step=136    -> 字符最右端刚到左沿
     *     step=144    -> 字符完全在左外, 停止
     */
    for (step = 0; step < (unsigned int)char_count * 8 + 8; step++) {
        /* 内层循环用的局部变量 (每步重新声明, C89 风格) */
        unsigned char row;           /* 屏幕行索引 0~7 */
        unsigned int  abs_col;       /* 整张超宽图中的列号 = step + 屏幕列 */
        unsigned int  char_pos;      /* abs_col 落在第几个字符上 */
        unsigned char char_col;      /* 字符内的列号 0~7 */
        const unsigned char __code *p;  /* 指向当前字符字模的 8 字节起始地址 */

        /* 清空 8 行显示缓冲区 (每行 1 字节)
         *   scroll_buf[0] = 第 0 行的 8 个 LED 状态
         *   scroll_buf[7] = 第 7 行的 8 个 LED 状态
         *   每步必须清空, 否则上一帧残留 */
        for (i = 0; i < 8; i++) {
            scroll_buf[i] = 0;
        }

        /* 遍历屏幕的 8 列 (0~7), 把超宽图对应的 8 列拼出来
         *
         *   屏幕 i=0     i=1     ...  i=7
         *   ┌─────┬─────┬─────┬─────┐
         *   │  c5 │  c5 │ ... │  c6 │   <-- 这些列来自超宽图的不同字符
         *   └─────┴─────┴─────┴─────┘
         *   abs_col= step+0  step+1 ... step+7
         */
        for (i = 0; i < 8; i++) {
            /* 屏幕的第 i 列, 在超宽图中的列号 = step + i */
            abs_col = step + i;

            /* 算出这个列号属于哪个字符, 以及在字符内的第几列
             *   例: abs_col = 25
             *     char_pos = 25 / 8 = 3  -> 第 4 个字符
             *     char_col = 25 % 8 = 1  -> 字符内的第 2 列
             *   即: 屏幕第 i 列 = FONT_DATA 中第 3 个字符的第 1 列 */
            char_pos = abs_col >> 3;          /* 除以 8, 相当于 abs_col / 8 */
            char_col = (unsigned char)(abs_col & 0x07);  /* 模 8, 相当于 abs_col % 8 */

            /* 越界检查: 如果 abs_col 超出字模范围, 该列不画
             *   例: 字模只有 17 字符 (列 0~135), 当 step=140 时
             *        abs_col=143 > 135, char_pos=17 >= 17, 跳过 */
            if (char_pos >= char_count) continue;

            /* 计算该字符字模在 font[] 里的起始地址
             *   例: char_pos=3, 起始地址 = &font[3 * 8] = &font[24]
             *   后续 8 字节就是第 4 个字符的 8 行图案 */
            p = &font[char_pos * 8];

            /* 把该字符的"char_col"列(1个像素)写到 scroll_buf 的"i"列(8 行)
             *
             *   例: p[3] = 0b01100000, char_col = 2
             *     0b01100000 & (0x80 >> 2) = 0b01100000 & 0b00100000 = 0b00100000 != 0
             *     -> 第 3 行的 i 列要亮
             *   scroll_buf[3] |= (0x80 >> i), 把第 3 行的 i 位置 1 */
            for (row = 0; row < 8; row++) {
                if (p[row] & (0x80 >> char_col)) {
                    scroll_buf[row] |= (unsigned char)(0x80 >> i);
                }
            }
        }

        /* 把拼好的 8 行图案送到显示驱动, 由 T0 中断每 1ms 扫描一次 */
        Matrix_Show(scroll_buf);

        /* 延时 speed_ms 毫秒, 这一帧停这么久, 然后 step+1 算下一帧
         *   延时期间 T0 中断照常, 屏幕上的图案一直显示 */
        ScrollDelayMs(speed_ms);
    }
}
