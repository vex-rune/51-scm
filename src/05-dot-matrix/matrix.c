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
 *   入参: row - 当前扫描行 (0~7)
 * ============================================================ */
static void ScanRow(unsigned char row)
{
    // 
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
     *   QA (D0) = 1 → 第 0 行不亮
     *   QB (D1) = 1 → 第 1 行不亮
     *   QC (D2) = 0 → 第 2 行亮 (阳极高)  ← 当前扫描行
     *   QD (D3) = 1 → 第 3 行不亮
     *   ... 其余 1
     *
     * 因为一帧只有 1 个 bit=0, 所以只有 1 行被点亮, 其他行熄灭
     */
    row_mask = (unsigned char)~(0x01 << row);

    // 获取低电平引脚值
    col_data = (unsigned char)~frame_buf[row];

    /* 先把列数据放上 P3, 等 595 锁存时一起生效 */
    P3 = col_data;

    /* 串行发送 row_mask 到 595, MSB 先发
     *
     * 时序: SCK 每次上升沿把 SER 的当前值推进移位寄存器
     * 发送顺序: bit7 → bit6 → ... → bit0
     *
     * 例 row_mask = 0b11111011:
     *   第 1 次: SER=1,  SCK↑  → 寄存器: 1....... (bit7 入)
     *   第 2 次: SER=1,  SCK↑  → 寄存器: 11...... (bit6 入)
     *   第 3 次: SER=1,  SCK↑  → 寄存器: 111..... (bit5 入)
     *   第 4 次: SER=1,  SCK↑  → 寄存器: 1111.... (bit4 入)
     *   第 5 次: SER=1,  SCK↑  → 寄存器: 11111... (bit3 入)
     *   第 6 次: SER=0,  SCK↑  → 寄存器: 111110.. (bit2=0 入) ← 当前行
     *   第 7 次: SER=1,  SCK↑  → 寄存器: 1111101. (bit1 入)
     *   第 8 次: SER=1,  SCK↑  → 寄存器: 11111011 (bit0 入) 完成
     */
    for (i = 0; i < 8; i++) {
        HC595_DATA = (row_mask & 0x80) ? 1 : 0;
        HC595_CLK = 0;
        row_mask <<= 1;
        HC595_CLK = 1;
    }

    /* 锁存: RCK 上升沿把 8 位移位寄存器的值打入输出锁存器
     * 此时 QA~QH 同时更新到 Q0~Q7 引脚, 点亮 row 行
     */
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
 *   - 9216 × 1.085μs ≈ 10ms? 错! 实际是 1ms (因为 12T 的具体分频)
 *   - 修正: 11.0592MHz / 12 / 9216 ≈ 1000Hz, 周期 = 1ms
 *
 * 调用流程 (硬件自动, 不是 C 代码调用):
 *   计数器溢出 → 硬件置 TF0=1
 *              → CPU 检查 EA=1 && ET0=1 (满足)
 *              → 硬件把 PC 压栈
 *              → CPU 跳转到 0x000B (T0 中断向量)
 *              → 0x000B 处的 LJMP 指令 → 跳到这里
 *
 * 注意:
 *   - 不能在 main.c 里手动调用 Timer0_ISR()
 *   - 函数末尾的 RETI 指令会硬件自动撤销中断优先级锁
 * ============================================================ */
void Timer0_ISR(void) __interrupt(1)
{
    /* 1. 扫描当前行 (根据 cur_row 算出 row_mask, 送到 595, 锁存) */
    ScanRow(cur_row);

    /* 2. cur_row 切换到下一行 (0→1→2→...→7→0 循环) */
    cur_row++;
    if (cur_row >= MATRIX_ROWS) {
        cur_row = 0;
    }

    /* 3. 重装定时初值 (模式1 不会自动重装, 必须手动)
     *    不重装的话, 下次会从 65536 累加, 周期变成 65536 × 1.085μs ≈ 71ms */
    TH0 = (65536 - 9216) / 256;
    TL0 = (65536 - 9216) % 256;
}
