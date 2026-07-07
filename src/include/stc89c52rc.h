/*
 * File:    stc89c52rc.h
 * Project: 公共头文件
 * Brief:   STC89C52RC 特殊功能寄存器 (SFR) 定义
 *
 * 8051 SFR 区域: 0x80-0xFF
 * 每个 SFR 占用 1 字节, 通过 SFR() 宏映射到内部 RAM 高 128 字节
 * 部分 SFR 中的特定位可通过 SBIT() 宏定义独立的位变量
 *
 * SFR 地址速查表 (按地址):
 *   0x80 P0    0x81 SP    0x82 DPL   0x83 DPH
 *   0x87 PCON  0x88 TCON  0x89 TMOD  0x8A TL0
 *   0x8B TL1   0x8C TH0   0x8D TH1
 *   0x8E AUXR  0x90 P1    0x92 ?     0x98 SCON
 *   0x99 SBUF  0xA0 P2    0xA2 AUXR1 0xA8 IE
 *   0xA9 SADDR 0xB0 P3    0xB7 IPH   0xB8 IP
 *   0xB9 SADEN 0xC0 XICON 0xC8 T2CON 0xC9 T2MOD
 *   0xCA RCAP2L 0xCB RCAP2H 0xCC TL2 0xCD TH2
 *   0xD0 PSW   0xD8 CCON  0xE0 ACC   0xE8 P4
 *   0xF0 B     0xF8 ?
 */

#ifndef STC89xx_H
#define STC89xx_H

#include <compiler.h>

// 适用于 STC89xx / STC90xx 系列的芯片
// Modified based on STC-ISP version by: ZnHoCn

/* The following is STC additional SFR*/

/*
 * #define _AUXR 0x8e
 * SFR(AUXR, 0x8e);
 * #define _AUXR1 0xa2
 * SFR(AUXR1, 0xa2);
 * #define _IPH 0xb7
 * SFR(IPH, 0xb7);
 */

/* P4 口 - STC 扩展 IO 端口, 8051 标准没有 P4
 * 注意: P4.5 兼作 ISP 下载的 ALE 引脚, 需要在烧录软件勾选
 * 默认映射:
 *   P40-P47 = P4.0-P4.7
 */
#define _P4 0xe8
SFR(P4, 0xe8);
SBIT(P46, _P4, 6);
SBIT(P45, _P4, 5);      // ISP下载需勾选"ALE脚用作P4.5口"
SBIT(P44, _P4, 4);
SBIT(P43, _P4, 3);
SBIT(P42, _P4, 2);
SBIT(P41, _P4, 1);
SBIT(P40, _P4, 0);

/* XICON - 扩展中断控制寄存器
 * 控制 P4.0/P4.1 等扩展外部中断源
 */
#define _XICON 0xc0
SFR(XICON, 0xc0);

/* 看门狗控制寄存器
 * WDT_CONTR - 控制看门狗定时器
 */
#define _WDT_CONTR 0xe1
SFR(WDT_CONTR, 0xe1);

/* ISP/IAP 编程相关寄存器
 * ISP_DATA   - 数据寄存器
 * ISP_ADDRH  - 地址高字节
 * ISP_ADDRL  - 地址低字节
 * ISP_CMD    - 命令寄存器
 * ISP_TRIG   - 触发寄存器
 * ISP_CONTR  - 控制寄存器
 */
#define _ISP_DATA 0xe2
SFR(ISP_DATA, 0xe2);
#define _ISP_ADDRH 0xe3
SFR(ISP_ADDRH, 0xe3);
#define _ISP_ADDRL 0xe4
SFR(ISP_ADDRL, 0xe4);
#define _ISP_CMD 0xe5
SFR(ISP_CMD, 0xe5);
#define _ISP_TRIG 0xe6
SFR(ISP_TRIG, 0xe6);
#define _ISP_CONTR 0xe7
SFR(ISP_CONTR, 0xe7);

/* Above is STC additional SFR */

/*--------------------------------------------------------------------------
REG51F.H

Header file for 8xC31/51, 80C51Fx, 80C51Rx+
Copyright (c) 1988-1999 Keil Elektronik GmbH and Keil Software, Inc.
All rights reserved.

Modification according to DataSheet from April 1999
 - SFR's AUXR and AUXR1 added for 80C51Rx+ derivatives
--------------------------------------------------------------------------*/

/*  通用 IO 端口寄存器
 * P0 (0x80): P0.0-P0.7, 8 位双向 IO, 内部无上拉 (使用需外接)
 * P1 (0x90): P1.0-P1.7, 8 位准双向 IO, 内部有上拉
 * P2 (0xA0): P2.0-P2.7, 8 位准双向 IO, 内部有上拉
 * P3 (0xB0): P3.0-P3.7, 8 位准双向 IO, 含第二功能 (RXD/TXD/INT/T 等)
 */
#define _P0 0x80
SFR(P0, 0x80);
SBIT(P00, _P0, 0);
SBIT(P01, _P0, 1);
SBIT(P02, _P0, 2);
SBIT(P03, _P0, 3);
SBIT(P04, _P0, 4);
SBIT(P05, _P0, 5);
SBIT(P06, _P0, 6);
SBIT(P07, _P0, 7);
#define _P1 0x90
SFR(P1, 0x90);
SBIT(P10, _P1, 0);
SBIT(P11, _P1, 1);
SBIT(P12, _P1, 2);
SBIT(P13, _P1, 3);
SBIT(P14, _P1, 4);
SBIT(P15, _P1, 5);
SBIT(P16, _P1, 6);
SBIT(P17, _P1, 7);
#define _P2 0xA0
SFR(P2, 0xA0);
SBIT(P20, _P2, 0);
SBIT(P21, _P2, 1);
SBIT(P22, _P2, 2);
SBIT(P23, _P2, 3);
SBIT(P24, _P2, 4);
SBIT(P25, _P2, 5);
SBIT(P26, _P2, 6);
SBIT(P27, _P2, 7);
#define _P3 0xB0
SFR(P3, 0xB0);

/* P3 第二功能映射 (默认是 IO, 配置后用作外设)
 *   P3.0 RXD  - 串口接收
 *   P3.1 TXD  - 串口发送
 *   P3.2 INT0 - 外部中断 0
 *   P3.3 INT1 - 外部中断 1
 *   P3.4 T0   - 定时器 0 外部输入
 *   P3.5 T1   - 定时器 1 外部输入
 *   P3.6 WR   - 外部数据存储器写
 *   P3.7 RD   - 外部数据存储器读
 */
SBIT(P30, _P3, 0);
SBIT(P31, _P3, 1);
SBIT(P32, _P3, 2);
SBIT(P33, _P3, 3);
SBIT(P34, _P3, 4);
SBIT(P35, _P3, 5);
SBIT(P36, _P3, 6);
SBIT(P37, _P3, 7);

/* 栈指针寄存器 SP (0x81)
 * 指向内部 RAM 中的栈顶, 复位后初值 0x07
 * 一般需要在 main 开头重置 SP 到较高地址 (如 0x60)
 */
#define _SP 0x81
SFR(SP, 0x81);

/* 数据指针寄存器 DPTR = DPH(高) + DPL(低)
 * 用于访问外部 RAM (XDATA) 或查表指令 MOVC
 */
#define _DPL 0x82
SFR(DPL, 0x82);
#define _DPH 0x83
SFR(DPH, 0x83);

/* PCON 电源控制寄存器 (0x87)
 * bit7 SMOD: 串口波特率倍速 (1=倍速)
 * bit6 -    : 保留
 * bit5 -    : 保留
 * bit4 -    : 保留
 * bit3 GF1  : 通用标志位
 * bit2 GF0  : 通用标志位
 * bit1 PD   : 掉电模式 (1=进入)
 * bit0 IDL  : 空闲模式 (1=进入)
 */
#define _PCON 0x87
SFR(PCON, 0x87);

/* TCON 定时器控制寄存器 (0x88)
 * bit7 TF1  : 定时器 1 溢出标志
 * bit6 TR1  : 定时器 1 运行控制 (1=启动)
 * bit5 TF0  : 定时器 0 溢出标志
 * bit4 TR0  : 定时器 0 运行控制
 * bit3 IE1  : 外部中断 1 请求标志
 * bit2 IT1  : 外部中断 1 触发方式 (1=下降沿, 0=低电平)
 * bit1 IE0  : 外部中断 0 请求标志
 * bit0 IT0  : 外部中断 0 触发方式
 */
#define _TCON 0x88
SFR(TCON, 0x88);

/* TMOD 定时器模式寄存器 (0x89)
 * 高 4 位控制 T1, 低 4 位控制 T0
 * 每组: M1 M0 C/T (GATE 不常用, 模式 0-3 配 M1 M0)
 * 模式 1 (M1=0, M0=1): 16 位定时器
 * 模式 2 (M1=1, M0=0): 8 位自动重装
 */
#define _TMOD 0x89
SFR(TMOD, 0x89);

/* 定时器初值寄存器
 * TL0/TH0 - 定时器 0 的低/高字节
 * TL1/TH1 - 定时器 1 的低/高字节
 * 模式 1 下 16 位计数, 需软件重装
 * 模式 2 下 TL 计数, TH 保存重载值
 */
#define _TL0 0x8A
SFR(TL0, 0x8A);
#define _TL1 0x8B
SFR(TL1, 0x8B);
#define _TH0 0x8C
SFR(TH0, 0x8C);
#define _TH1 0x8D
SFR(TH1, 0x8D);

/* IE 中断允许寄存器 (0xA8)
 * bit7 EA   : 总中断允许 (1=开)
 * bit6 EC   : PCA 中断允许
 * bit5 ET2  : 定时器 2 中断允许
 * bit4 ES   : 串口中断允许
 * bit3 ET1  : 定时器 1 中断允许
 * bit2 EX1  : 外部中断 1 允许
 * bit1 ET0  : 定时器 0 中断允许
 * bit0 EX0  : 外部中断 0 允许
 */
#define _IE 0xA8
SFR(IE, 0xA8);

/* IP 中断优先级寄存器 (0xB8)
 * bit5 PT2  : 定时器 2 优先级
 * bit4 PS   : 串口优先级
 * bit3 PT1  : 定时器 1 优先级
 * bit2 PX1  : 外部中断 1 优先级
 * bit1 PT0  : 定时器 0 优先级
 * bit0 PX0  : 外部中断 0 优先级
 * 1=高优先级, 0=低优先级 (允许嵌套)
 */
#define _IP 0xB8
SFR(IP, 0xB8);

/* 串口相关寄存器
 * SCON (0x98) 串口控制寄存器
 *   bit7 SM0/FE: 模式高位/帧错误 (与 SMOD 配置相关)
 *   bit6 SM1   : 模式低位 (常用 01 = 8 位 UART, 模式 1)
 *   bit5 SM2   : 多机通信使能
 *   bit4 REN   : 接收允许 (1=允许)
 *   bit3 TB8   : 第 9 位发送数据
 *   bit2 RB8   : 第 9 位接收数据
 *   bit1 TI    : 发送完成标志 (需软件清 0)
 *   bit0 RI    : 接收完成标志 (需软件清 0)
 * SBUF (0x99) 串口数据寄存器, 写=发送, 读=接收
 */
#define _SCON 0x98
SFR(SCON, 0x98);
#define _SBUF 0x99
SFR(SBUF, 0x99);

/*  80C51Fx/Rx Extensions  */

/* AUXR 辅助寄存器 (0x8E) - STC 增强功能
 * bit7 T0x12 : 定时器 0 速度控制 (1=不分频, 0=12 分频)
 * bit6 T1x12 : 定时器 1 速度控制
 * bit5 UART_M0x6 : 串口模式 0 波特率控制 (1=6 分频即 fosc/6)
 * bit4 T2R   : 定时器 2 运行控制 (部分 STC 型号)
 * bit3 T2CT  : 定时器 2 计数器/定时器选择
 * bit2 T2x12 : 定时器 2 速度控制
 * bit1 EXTRAM: 外部 RAM 使能
 * bit0 S1BRS : 串口 1 波特率发生器选择 (1=T2, 0=T1)
 */
#define _AUXR 0x8E
SFR(AUXR, 0x8E);

/* AUXR1 辅助寄存器 1 (0xA2)
 * bit7 -     : 保留
 * bit6 -     : 保留
 * bit5 -     : 保留
 * bit4 -     : 保留
 * bit3 -     : 保留
 * bit2 -     : 保留
 * bit1 -     : 保留
 * bit0 DPS   : 数据指针选择 (1=DPTR1, 0=DPTR0)
 */
#define _AUXR1 0xA2
SFR(AUXR1, 0xA2);

#define _SADDR 0xA9
SFR(SADDR, 0xA9);
#define _IPH 0xB7
SFR(IPH, 0xB7);
#define _SADEN 0xB9
SFR(SADEN, 0xB9);
#define _T2CON 0xC8
SFR(T2CON, 0xC8);
#define _T2MOD 0xC9
SFR(T2MOD, 0xC9);
#define _RCAP2L 0xCA
SFR(RCAP2L, 0xCA);
#define _RCAP2H 0xCB
SFR(RCAP2H, 0xCB);
#define _TL2 0xCC
SFR(TL2, 0xCC);
#define _TH2 0xCD
SFR(TH2, 0xCD);

/* PCA SFR
#define _CCON 0xD8
SFR(CCON, 0xD8);
#define _CMOD 0xD9
SFR(CMOD, 0xD9);
#define _CCAPM0 0xDA
SFR(CCAPM0, 0xDA);
#define _CCAPM1 0xDB
SFR(CCAPM1, 0xDB);
#define _CCAPM2 0xDC
SFR(CCAPM2, 0xDC);
#define _CCAPM3 0xDD
SFR(CCAPM3, 0xDD);
#define _CCAPM4 0xDE
SFR(CCAPM4, 0xDE);
#define _CL 0xE9
SFR(CL, 0xE9);
#define _CCAP0L 0xEA
SFR(CCAP0L, 0xEA);
#define _CCAP1L 0xEB
SFR(CCAP1L, 0xEB);
#define _CCAP2L 0xEC
SFR(CCAP2L, 0xEC);
#define _CCAP3L 0xED
SFR(CCAP3L, 0xED);
#define _CCAP4L 0xEE
SFR(CCAP4L, 0xEE);
#define _CH 0xF9
SFR(CH, 0xF9);
#define _CCAP0H 0xFA
SFR(CCAP0H, 0xFA);
#define _CCAP1H 0xFB
SFR(CCAP1H, 0xFB);
#define _CCAP2H 0xFC
SFR(CCAP2H, 0xFC);
#define _CCAP3H 0xFD
SFR(CCAP3H, 0xFD);
#define _CCAP4H 0xFE
SFR(CCAP4H, 0xFE);
*/

/*  状态与累加器
 * PSW (0xD0) 程序状态字
 *   bit7 CY  : 进位标志
 *   bit6 AC  : 辅助进位 (半字节)
 *   bit5 F0  : 用户自定义标志
 *   bit4 RS1 : 工作寄存器组选择高位
 *   bit3 RS0 : 工作寄存器组选择低位
 *   bit2 OV  : 溢出标志
 *   bit1 -   : 保留
 *   bit0 P   : 奇偶标志
 * ACC (0xE0) 累加器, 多数算术逻辑运算的源和目标
 * B  (0xF0) 乘除法辅助寄存器
 */
#define _PSW 0xD0
SFR(PSW, 0xD0);
#define _ACC 0xE0
SFR(ACC, 0xE0);
#define _B 0xF0
SFR(B, 0xF0);

/*  状态位
 * PSW 各位 */
SBIT(CY, _PSW, 7);
SBIT(AC, _PSW, 6);
SBIT(F0, _PSW, 5);
SBIT(RS1, _PSW, 4);
SBIT(RS0, _PSW, 3);
SBIT(OV, _PSW, 2);
SBIT(P, _PSW, 0);

/* TCON 各位 */
SBIT(TF1, _TCON, 7);
SBIT(TR1, _TCON, 6);
SBIT(TF0, _TCON, 5);
SBIT(TR0, _TCON, 4);
SBIT(IE1, _TCON, 3);
SBIT(IT1, _TCON, 2);
SBIT(IE0, _TCON, 1);
SBIT(IT0, _TCON, 0);

/* IE 各位
 * EA  - 总中断开关
 * EC  - PCA 中断 (一般不用)
 * ET2 - 定时器 2
 * ES  - 串口
 * ET1 - 定时器 1
 * EX1 - 外部中断 1
 * ET0 - 定时器 0
 * EX0 - 外部中断 0
 */
SBIT(EA, _IE, 7);
SBIT(EC, _IE, 6);
SBIT(ET2, _IE, 5);
SBIT(ES, _IE, 4);
SBIT(ET1, _IE, 3);
SBIT(EX1, _IE, 2);
SBIT(ET0, _IE, 1);
SBIT(EX0, _IE, 0);

/* IP 各位
 * PT2 - 定时器 2 优先级
 * PS  - 串口优先级
 * PT1 - 定时器 1 优先级
 * PX1 - 外部中断 1 优先级
 * PT0 - 定时器 0 优先级
 * PX0 - 外部中断 0 优先级
 */
/*  SBIT(PPC, _IP, 6);*/
SBIT(PT2, _IP, 5);
SBIT(PS, _IP, 4);
SBIT(PT1, _IP, 3);
SBIT(PX1, _IP, 2);
SBIT(PT0, _IP, 1);
SBIT(PX0, _IP, 0);

/* P3 各位第二功能
 * RD/P3.7 - 外部 RAM 读选通
 * WR/P3.6 - 外部 RAM 写选通
 * T1/P3.5 - 定时器 1 外部输入 (也用作定时器 0 中断标志)
 * T0/P3.4 - 定时器 0 外部输入
 * INT1/P3.3 - 外部中断 1
 * INT0/P3.2 - 外部中断 0
 * TXD/P3.1 - 串口发送
 * RXD/P3.0 - 串口接收
 */
SBIT(RD, _P3, 7);
SBIT(WR, _P3, 6);
SBIT(T1, _P3, 5);
SBIT(T0, _P3, 4);
SBIT(INT1, _P3, 3);
SBIT(INT0, _P3, 2);
SBIT(TXD, _P3, 1);
SBIT(RXD, _P3, 0);

/* SCON 各位
 * SM0/FE - 模式高位 / 帧错误标志
 * SM1    - 模式低位
 * SM2    - 停止位校验
 * REN    - 接收允许
 * TB8    - 第 9 位发送数据
 * RB8    - 第 9 位接收数据
 * TI     - 发送完成中断标志 (软件清 0)
 * RI     - 接收完成中断标志 (软件清 0)
 */
SBIT(SM0, _SCON, 7);        // alternatively "FE"
SBIT(FE, _SCON, 7);
SBIT(SM1, _SCON, 6);
SBIT(SM2, _SCON, 5);
SBIT(REN, _SCON, 4);
SBIT(TB8, _SCON, 3);
SBIT(RB8, _SCON, 2);
SBIT(TI, _SCON, 1);
SBIT(RI, _SCON, 0);

/*  P1  */
/* PCA
SBIT(CEX4, _P1, 7);
SBIT(CEX3, _P1, 6);
SBIT(CEX2, _P1, 5);
SBIT(CEX1, _P1, 4);
SBIT(CEX0, _P1, 3);
SBIT(ECI, _P1, 2);
*/

SBIT(T2EX, _P1, 1);
SBIT(T2, _P1, 0);

/*  T2CON 各位 - 定时器 2 控制
 * TF2    - 定时器 2 溢出
 * EXF2   - 定时器 2 外部标志
 * RCLK   - 接收时钟选择 (1=T2 产生)
 * TCLK   - 发送时钟选择
 * EXEN2  - 外部使能
 * TR2    - 运行控制
 * C_T2   - 计数器/定时器选择
 * CP_RL2 - 捕获/重装选择
 */
SBIT(TF2, _T2CON, 7);
SBIT(EXF2, _T2CON, 6);
SBIT(RCLK, _T2CON, 5);
SBIT(TCLK, _T2CON, 4);
SBIT(EXEN2, _T2CON, 3);
SBIT(TR2, _T2CON, 2);
SBIT(C_T2, _T2CON, 1);
SBIT(CP_RL2, _T2CON, 0);

/*  CCON  */
/* PCA
SBIT(CF, _CCON, 6);
SBIT(CR, _CCON, 6);

SBIT(CCF4, _CCON, 4);
SBIT(CCF3, _CCON, 3);
SBIT(CCF2, _CCON, 2);
SBIT(CCF1, _CCON, 1);
SBIT(CCF0, _CCON, 0);
*/

#endif
