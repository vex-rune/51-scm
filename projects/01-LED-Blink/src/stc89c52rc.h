/*
 * File:    stc89c52rc.h
 * Project: 01-LED-Blink
 * MCU:     STC89C52RC (8051 内核)
 * Tool:    PlatformIO + SDCC
 * Brief:   补全 SDCC mcs51/8051.h 中缺失的 STC89C52RC 增强型 SFR
 *
 * 注意：SDCC 的 mcs51/8051.h 已定义标准 8051 SFR (P0~P3, TCON, TMOD,
 *       SCON, IE, IP, PSW, ACC, B, SP, DPTR 等) 及部分 sbit。
 *       此文件只补 SDCC 缺失的部分，避免重复定义导致编译错误。
 */
#ifndef __STC89C52RC_H__
#define __STC89C52RC_H__

/* 依赖：先包含 SFR 定义，再包含 intrinsics (_nop_ 等) */
#include <mcs51/8051.h>
#include <mcs51/compiler.h>

/* ============================================================
 * 补全标准 8051 SFR（mcs51/8051.h 可能缺失的）
 * ============================================================ */

/* P0 ~ P3 端口（SDCC 未定义） */
#ifndef P0
__sfr __at(0x80) P0;
#endif
#ifndef P1
__sfr __at(0x90) P1;
#endif
#ifndef P2
__sfr __at(0xA0) P2;
#endif
#ifndef P3
__sfr __at(0xB0) P3;
#endif

/* 定时器 */
#ifndef TMOD
__sfr __at(0x89) TMOD;
#endif
#ifndef TL0
__sfr __at(0x8A) TL0;
#endif
#ifndef TL1
__sfr __at(0x8B) TL1;
#endif
#ifndef TH0
__sfr __at(0x8C) TH0;
#endif
#ifndef TH1
__sfr __at(0x8D) TH1;
#endif

/* 串口 */
#ifndef SCON
__sfr __at(0x98) SCON;
#endif
#ifndef SBUF
__sfr __at(0x99) SBUF;
#endif
#ifndef PCON
__sfr __at(0x87) PCON;
#endif

/* ============================================================
 * 标准 8051 sbit（mcs51/8051.h 可能缺失的，用 #ifndef 保护）
 * ============================================================ */

#ifndef P1_0
__sbit __at(0x90) P1_0;
#endif
#ifndef P1_1
__sbit __at(0x91) P1_1;
#endif
#ifndef P1_2
__sbit __at(0x92) P1_2;
#endif
#ifndef P1_3
__sbit __at(0x93) P1_3;
#endif
#ifndef P1_4
__sbit __at(0x94) P1_4;
#endif
#ifndef P1_5
__sbit __at(0x95) P1_5;
#endif
#ifndef P1_6
__sbit __at(0x96) P1_6;
#endif
#ifndef P1_7
__sbit __at(0x97) P1_7;
#endif

#ifndef P3_0
__sbit __at(0xB0) P3_0;  /* RXD */
#endif
#ifndef P3_1
__sbit __at(0xB1) P3_1;  /* TXD */
#endif
#ifndef P3_2
__sbit __at(0xB2) P3_2;  /* INT0 */
#endif
#ifndef P3_3
__sbit __at(0xB3) P3_3;  /* INT1 */
#endif
#ifndef P3_4
__sbit __at(0xB4) P3_4;  /* T0 */
#endif
#ifndef P3_5
__sbit __at(0xB5) P3_5;  /* T1 */
#endif
#ifndef P3_6
__sbit __at(0xB6) P3_6;  /* WR */
#endif
#ifndef P3_7
__sbit __at(0xB7) P3_7;  /* RD */
#endif

/* 串口 sbit */
#ifndef RI
__sbit __at(0x98) RI;
#endif
#ifndef TI
__sbit __at(0x99) TI;
#endif
#ifndef RB8
__sbit __at(0x9A) RB8;
#endif
#ifndef TB8
__sbit __at(0x9B) TB8;
#endif
#ifndef REN
__sbit __at(0x9C) REN;
#endif
#ifndef SM2
__sbit __at(0x9D) SM2;
#endif
#ifndef SM0
__sbit __at(0x9F) SM0;
#endif

/* 中断使能 sbit */
#ifndef ET2
__sbit __at(0xAD) ET2;   /* 定时器 2 中断使能（STC 增强型） */
#endif

/* 中断优先级 sbit */
#ifndef PT2
__sbit __at(0xBD) PT2;
#endif

/* PSW 位 */
#ifndef RS0
__sbit __at(0xD3) RS0;
#endif
#ifndef RS1
__sbit __at(0xD4) RS1;
#endif

/* ============================================================
 * STC89C52RC 增强型 SFR（SDCC 完全缺失）
 * ============================================================ */

/* 定时器 2 */
__sfr __at(0xC8) T2CON;
__sfr __at(0xCA) RCAP2L;
__sfr __at(0xCB) RCAP2H;
__sfr __at(0xCC) TL2;
__sfr __at(0xCD) TH2;

/* 定时器 2 sbit */
__sbit __at(0xC8) CP_RL2;
__sbit __at(0xC9) C_T2;
__sbit __at(0xCA) TR2;
__sbit __at(0xCB) EXEN2;
__sbit __at(0xCC) TCLK;
__sbit __at(0xCD) RCLK;
__sbit __at(0xCE) EXF2;
__sbit __at(0xCF) TF2;

/* 辅助寄存器 */
__sfr __at(0x8E) AUXR;
__sfr __at(0x97) AUXR2;

/* 双串口（STC 增强型） */
__sfr __at(0x9A) S2CON;
__sfr __at(0x9B) S2BUF;

/* 看门狗 */
__sfr __at(0xC1) WDT_CONTR;

/* ISP/IAP */
__sfr __at(0xE2) ISP_DATA;
__sfr __at(0xE3) ISP_ADDRH;
__sfr __at(0xE4) ISP_ADDRL;
__sfr __at(0xE5) ISP_CMD;
__sfr __at(0xE6) ISP_TRIG;
__sfr __at(0xE7) ISP_CONTR;

/* 扩展端口 */
__sfr __at(0xC0) P4;
__sfr __at(0xC8) P4SW;
__sfr __at(0xE8) P5;

/* 中断优先级扩展 */
__sfr __at(0xBF) IPH;

/* ============================================================
 * 常用宏
 * ============================================================ */
#define BIT(n)       (1 << (n))
#define EA_ENABLE()  do { EA = 1; } while (0)
#define EA_DISABLE() do { EA = 0; } while (0)

#endif /* __STC89C52RC_H__ */