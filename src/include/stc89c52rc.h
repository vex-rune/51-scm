/*
 * ============================================================
 * File:    stc89c52rc.h
 * MCU:     STC89C52RC (8051 内核, 8KB Flash, 512B RAM)
 * Tool:    SDCC (PlatformIO)
 * Brief:   STC89C52RC 完整 SFR 寄存器地址映射
 *
 * 数据来源: STC89C51RC/RD+ Data Sheet (www.stcmicro.com)
 *           SDCC mcs51/8052.h, laborer/c51drv
 *
 * 注意: SDCC 中 __sfr / __sbit 用法与 Keil 不同。
 *       __sfr __at(addr) 定义 8 位 SFR
 *       __sbit __at(addr) 定义可位寻址 SFR 的位
 *       位地址 = 寄存器字节地址 + 位偏移 (仅适用于 0x80/0x88/0x90...)
 * ============================================================
 */

#ifndef __STC89C52RC_H__
#define __STC89C52RC_H__

/* 跨编译器兼容层: 提供 SFR()/SBIT()/NOP() 等宏, 屏蔽 SDCC/Keil/IAR 语法差异
   当前项目使用 SDCC 原生 __sfr/__sbit 语法, 未调用该文件中的宏 */
#include <mcs51/compiler.h>

/* ============================================================
 * 一、CPU 核心 SFR（字节地址排序）
 *    标 * 号的表示 SDCC 8051.h 中可能已定义, 用 #ifndef 保护
 * ============================================================ */

/* ---- 0x80: P0 端口（可位寻址） ---- */
#ifndef P0
__sfr __at(0x80) P0;
#endif

/* ---- 0x81: 堆栈指针 ---- */
#ifndef SP
__sfr __at(0x81) SP;
#endif

/* ---- 0x82: 数据指针低字节 ---- */
#ifndef DPL
__sfr __at(0x82) DPL;
#endif

/* ---- 0x83: 数据指针高字节 ---- */
#ifndef DPH
__sfr __at(0x83) DPH;
#endif

/* ---- 0x87: 电源控制 ---- */
#ifndef PCON
__sfr __at(0x87) PCON;
#endif

/* ---- 0x88: 定时器控制（可位寻址）---- */
#ifndef TCON
__sfr __at(0x88) TCON;
#endif

/* ---- 0x89: 定时器模式 ---- */
#ifndef TMOD
__sfr __at(0x89) TMOD;
#endif

/* ---- 0x8A: 定时器0低字节 ---- */
#ifndef TL0
__sfr __at(0x8A) TL0;
#endif

/* ---- 0x8B: 定时器1低字节 ---- */
#ifndef TL1
__sfr __at(0x8B) TL1;
#endif

/* ---- 0x8C: 定时器0高字节 ---- */
#ifndef TH0
__sfr __at(0x8C) TH0;
#endif

/* ---- 0x8D: 定时器1高字节 ---- */
#ifndef TH1
__sfr __at(0x8D) TH1;
#endif

/* ---- 0x8E: 辅助寄存器（STC 扩展）----
   bit7: -       bit6: -
   bit5: -       bit4: -
   bit3: -       bit2: -
   bit1: EXTRAM  0=内部扩展RAM可用, 1=禁止
   bit0: ALEOFF  0=ALE固定输出, 1=只在MOVX/MOVC时输出 */
__sfr __at(0x8E) AUXR;

/* ---- 0x90: P1 端口（可位寻址）---- */
#ifndef P1
__sfr __at(0x90) P1;
#endif

/* ---- 0x98: 串口控制（可位寻址）---- */
#ifndef SCON
__sfr __at(0x98) SCON;
#endif

/* ---- 0x99: 串口数据缓冲 ---- */
#ifndef SBUF
__sfr __at(0x99) SBUF;
#endif

/* ---- 0x9A: 第二串口控制（STC 扩展, 可位寻址）---- */
__sfr __at(0x9A) S2CON;

/* ---- 0x9B: 第二串口数据缓冲（STC 扩展）---- */
__sfr __at(0x9B) S2BUF;

/* ---- 0xA0: P2 端口（可位寻址）---- */
#ifndef P2
__sfr __at(0xA0) P2;
#endif

/* ---- 0xA2: 辅助寄存器1（STC 扩展）----
   bit7: -       bit6: -
   bit5: -       bit4: -
   bit3: -       bit2: -
   bit1: -       bit0: DPS   0=DPTR0, 1=DPTR1 (双DPTR切换) */
__sfr __at(0xA2) AUXR1;

/* ---- 0xA8: 中断使能（可位寻址）---- */
#ifndef IE
__sfr __at(0xA8) IE;
#endif

/* ---- 0xA9: 从机地址（STC 扩展）---- */
__sfr __at(0xA9) SADDR;

/* ---- 0xB0: P3 端口（可位寻址）---- */
#ifndef P3
__sfr __at(0xB0) P3;
#endif

/* ---- 0xB7: 高中断优先级（STC 扩展, 4级优先级的高2位）---- */
__sfr __at(0xB7) IPH;

/* ---- 0xB8: 中断优先级（可位寻址）---- */
#ifndef IP
__sfr __at(0xB8) IP;
#endif

/* ---- 0xB9: 从机地址掩码（STC 扩展）---- */
__sfr __at(0xB9) SADEN;

/* ---- 0xC0: 扩展中断控制（STC 扩展, 可位寻址）----
   bit7: PX3     bit6: EX3     bit5: IE3     bit4: IT3
   bit3: PX2     bit2: EX2     bit1: IE2     bit0: IT2
   管理外部中断 INT2(P4.3) 和 INT3(P4.2) */
__sfr __at(0xC0) XICON;

/* ---- 0xC8: 定时器2控制（可位寻址）---- */
#ifndef T2CON
__sfr __at(0xC8) T2CON;
#endif

/* ---- 0xC9: 定时器2模式（STC 扩展）---- */
__sfr __at(0xC9) T2MOD;

/* ---- 0xCA: 定时器2 捕获/重装低字节 ---- */
#ifndef RCAP2L
__sfr __at(0xCA) RCAP2L;
#endif

/* ---- 0xCB: 定时器2 捕获/重装高字节 ---- */
#ifndef RCAP2H
__sfr __at(0xCB) RCAP2H;
#endif

/* ---- 0xCC: 定时器2 低字节 ---- */
#ifndef TL2
__sfr __at(0xCC) TL2;
#endif

/* ---- 0xCD: 定时器2 高字节 ---- */
#ifndef TH2
__sfr __at(0xCD) TH2;
#endif

/* ---- 0xD0: 程序状态字（可位寻址）---- */
#ifndef PSW
__sfr __at(0xD0) PSW;
#endif

/* ---- 0xE0: 累加器（可位寻址）---- */
#ifndef ACC
__sfr __at(0xE0) ACC;
#endif

/* ---- 0xE1: 看门狗控制（STC 扩展）----
   bit7: WDT_FLAG  看门狗溢出标志
   bit6: -         bit5: EN_WDT    使能位
   bit4: CLR_WDT   清0计数器(喂狗)
   bit3: IDL_WDT   空闲模式是否计数
   bit2: PS2       bit1: PS1       bit0: PS0  预分频 */
__sfr __at(0xE1) WDT_CONTR;

/* ---- 0xE2: ISP/IAP 数据寄存器（STC 扩展）---- */
__sfr __at(0xE2) ISP_DATA;

/* ---- 0xE3: ISP/IAP 地址高字节（STC 扩展）---- */
__sfr __at(0xE3) ISP_ADDRH;

/* ---- 0xE4: ISP/IAP 地址低字节（STC 扩展）---- */
__sfr __at(0xE4) ISP_ADDRL;

/* ---- 0xE5: ISP/IAP 命令寄存器（STC 扩展）----
   0=空闲, 1=读, 2=写, 3=擦除 */
__sfr __at(0xE5) ISP_CMD;

/* ---- 0xE6: ISP/IAP 触发寄存器（STC 扩展）----
   先写 0x46, 再写 0xB9 触发操作 */
__sfr __at(0xE6) ISP_TRIG;

/* ---- 0xE7: ISP/IAP 控制寄存器（STC 扩展）----
   bit7: ISPEN    bit6: SWBS     bit5: SWRST
   bit4: -        bit2~0: WT2~WT0  等待时间 */
__sfr __at(0xE7) ISP_CONTR;

/* ---- 0xE8: P4 端口（STC 扩展, 可位寻址）----
   仅 LQFP-44/PLCC-44 封装可用, PDIP-40 无此端口
   P4.3 = INT2,  P4.2 = INT3 */
__sfr __at(0xE8) P4;

/* ---- 0xF0: B 寄存器（可位寻址）---- */
#ifndef B
__sfr __at(0xF0) B;
#endif


/* ============================================================
 * 二、SFR 位定义（sbit, 仅定义 SDCC 中可能缺失的）
 *    标准 8051 的 sbit 由 SDCC 8051.h 提供, 此处不再重复
 * ============================================================ */

/* -- TCON 位（0x88）-- */
#ifndef IT0
__sbit __at(0x88) IT0;   /* 外部中断0触发方式 */
#endif
#ifndef IE0
__sbit __at(0x89) IE0;   /* 外部中断0标志 */
#endif
#ifndef IT1
__sbit __at(0x8A) IT1;   /* 外部中断1触发方式 */
#endif
#ifndef IE1
__sbit __at(0x8B) IE1;   /* 外部中断1标志 */
#endif
#ifndef TR0
__sbit __at(0x8C) TR0;   /* 定时器0启停 */
#endif
#ifndef TF0
__sbit __at(0x8D) TF0;   /* 定时器0溢出标志 */
#endif
#ifndef TR1
__sbit __at(0x8E) TR1;   /* 定时器1启停 */
#endif
#ifndef TF1
__sbit __at(0x8F) TF1;   /* 定时器1溢出标志 */
#endif

/* -- P1 端口位（0x90~0x97）-- */
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

/* P1 复用功能 */
__sbit __at(0x90) T2;      /* P1.0 = T2  定时器2外部输入 */
__sbit __at(0x91) T2EX;    /* P1.1 = T2EX 定时器2捕获/重装触发 */

/* -- SCON 位（0x98~0x9F）-- */
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
#ifndef SM1
__sbit __at(0x9E) SM1;
#endif
#ifndef SM0
__sbit __at(0x9F) SM0;
#endif

/* S2CON 位（STC 扩展, 0x9A~0x9F 与 SCON 共享字节地址不冲突） */
/* 注: S2CON 寄存器为 0x9A, 其位地址也是 0x9A~0x9F 但作用于不同寄存器 */
/* SDCC 不支持同名位地址的二义性, 此处仅给出 S2CON 的位宏 */
#define S2RI   0x01   /* 接收中断 */
#define S2TI   0x02   /* 发送中断 */
#define S2RB8  0x04   /* 接收第9位 */
#define S2TB8  0x08   /* 发送第9位 */
#define S2REN  0x10   /* 接收使能 */
#define S2SM2  0x20   /* 多机通信 */
#define S2SM0  0x80   /* 模式选择 */

/* -- P2 端口位（0xA0~0xA7）-- */
#ifndef P2_0
__sbit __at(0xA0) P2_0;
#endif
#ifndef P2_1
__sbit __at(0xA1) P2_1;
#endif
#ifndef P2_2
__sbit __at(0xA2) P2_2;
#endif
#ifndef P2_3
__sbit __at(0xA3) P2_3;
#endif
#ifndef P2_4
__sbit __at(0xA4) P2_4;
#endif
#ifndef P2_5
__sbit __at(0xA5) P2_5;
#endif
#ifndef P2_6
__sbit __at(0xA6) P2_6;
#endif
#ifndef P2_7
__sbit __at(0xA7) P2_7;
#endif

/* -- IE 位（0xA8~0xAF）-- */
#ifndef EX0
__sbit __at(0xA8) EX0;   /* INT0 中断使能 */
#endif
#ifndef ET0
__sbit __at(0xA9) ET0;   /* 定时器0中断使能 */
#endif
#ifndef EX1
__sbit __at(0xAA) EX1;   /* INT1 中断使能 */
#endif
#ifndef ET1
__sbit __at(0xAB) ET1;   /* 定时器1中断使能 */
#endif
#ifndef ES
__sbit __at(0xAC) ES;    /* 串口中断使能 */
#endif
#ifndef ET2
__sbit __at(0xAD) ET2;   /* 定时器2中断使能（8052） */
#endif
#ifndef EA
__sbit __at(0xAF) EA;    /* 总中断使能 */
#endif

/* -- P3 端口位（0xB0~0xB7）-- */
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

/* -- IP 位（0xB8~0xBF）-- */
#ifndef PX0
__sbit __at(0xB8) PX0;
#endif
#ifndef PT0
__sbit __at(0xB9) PT0;
#endif
#ifndef PX1
__sbit __at(0xBA) PX1;
#endif
#ifndef PT1
__sbit __at(0xBB) PT1;
#endif
#ifndef PS
__sbit __at(0xBC) PS;
#endif
#ifndef PT2
__sbit __at(0xBD) PT2;   /* 定时器2优先级（8052） */
#endif

/* -- T2CON 位（0xC8~0xCF）-- */
__sbit __at(0xC8) CP_RL2;  /* 捕获/重装选择 */
__sbit __at(0xC9) C_T2;    /* 计数/定时选择 */
__sbit __at(0xCA) TR2;     /* 定时器2启停 */
__sbit __at(0xCB) EXEN2;   /* 外部使能 */
__sbit __at(0xCC) TCLK;    /* 发送时钟选择 */
__sbit __at(0xCD) RCLK;    /* 接收时钟选择 */
__sbit __at(0xCE) EXF2;    /* 外部标志 */
__sbit __at(0xCF) TF2;     /* 溢出标志 */

/* -- XICON 位（STC 扩展, 0xC0~0xC7）-- */
__sbit __at(0xC0) IT2;     /* INT2 触发方式 */
__sbit __at(0xC1) IE2;     /* INT2 标志 */
__sbit __at(0xC2) EX2;     /* INT2 使能 */
__sbit __at(0xC3) PX2;     /* INT2 优先级 */
__sbit __at(0xC4) IT3;     /* INT3 触发方式 */
__sbit __at(0xC5) IE3;     /* INT3 标志 */
__sbit __at(0xC6) EX3;     /* INT3 使能 */
__sbit __at(0xC7) PX3;     /* INT3 优先级 */

/* -- PSW 位（0xD0~0xD7）-- */
#ifndef P
__sbit __at(0xD0) P;       /* 奇偶标志 */
#endif
#ifndef F1
__sbit __at(0xD1) F1;      /* 用户标志1 */
#endif
#ifndef OV
__sbit __at(0xD2) OV;      /* 溢出标志 */
#endif
#ifndef RS0
__sbit __at(0xD3) RS0;     /* 寄存器组选择0 */
#endif
#ifndef RS1
__sbit __at(0xD4) RS1;     /* 寄存器组选择1 */
#endif
#ifndef F0
__sbit __at(0xD5) F0;      /* 用户标志0 */
#endif
#ifndef AC
__sbit __at(0xD6) AC;      /* 辅助进位 */
#endif
#ifndef CY
__sbit __at(0xD7) CY;      /* 进位标志 */
#endif

/* -- P4 端口位（STC 扩展, 0xE8~0xEB, 仅4位）-- */
__sbit __at(0xE8) P4_0;
__sbit __at(0xE9) P4_1;
__sbit __at(0xEA) P4_2;    /* INT3 */
__sbit __at(0xEB) P4_3;    /* INT2 */

/* ============================================================
 * 三、常用操作宏
 * ============================================================ */
#define BIT(n)       (1 << (n))
#define EA_ENABLE()  do { EA = 1; } while (0)
#define EA_DISABLE() do { EA = 0; } while (0)

/* ISP/IAP 命令 */
#define ISP_IDLE    0   /* 空闲 */
#define ISP_READ    1   /* 读 */
#define ISP_WRITE   2   /* 写 */
#define ISP_ERASE   3   /* 扇区擦除 */

/* 看门狗预分频（PS2:PS0, 12T @ 12MHz） */
#define WDT_PS_2    0x00   /* 预分频2   ≈ 65ms */
#define WDT_PS_4    0x01   /* 预分频4   ≈ 131ms */
#define WDT_PS_8    0x02   /* 预分频8   ≈ 262ms */
#define WDT_PS_16   0x03   /* 预分频16  ≈ 524ms */
#define WDT_PS_32   0x04   /* 预分频32  ≈ 1.05s */
#define WDT_PS_64   0x05   /* 预分频64  ≈ 2.10s */
#define WDT_PS_128  0x06   /* 预分频128 ≈ 4.19s */
#define WDT_PS_256  0x07   /* 预分频256 ≈ 8.39s */

/* ============================================================
 * 四、SFR 地址速查表
 *
 *  地址    寄存器      类别              可位寻址
 *  ─────  ──────────  ────────────────  ────────
 *  0x80   P0          I/O端口            是
 *  0x81   SP          堆栈指针            否
 *  0x82   DPL         数据指针低         否
 *  0x83   DPH         数据指针高         否
 *  0x87   PCON        电源控制            否
 *  0x88   TCON        定时器控制          是
 *  0x89   TMOD        定时器模式          否
 *  0x8A   TL0         定时器0低           否
 *  0x8B   TL1         定时器1低           否
 *  0x8C   TH0         定时器0高           否
 *  0x8D   TH1         定时器1高           否
 *  0x8E   AUXR        辅助(STC扩展)       否
 *  0x90   P1          I/O端口             是
 *  0x98   SCON        串口控制             是
 *  0x99   SBUF        串口数据             否
 *  0x9A   S2CON       第二串口控制(STC)    否*
 *  0x9B   S2BUF       第二串口数据(STC)    否
 *  0xA0   P2          I/O端口              是
 *  0xA2   AUXR1       辅助1(STC扩展)       否
 *  0xA8   IE          中断使能             是
 *  0xA9   SADDR       从机地址(STC)        否
 *  0xB0   P3          I/O端口              是
 *  0xB7   IPH         高中断优先级(STC)    否
 *  0xB8   IP          中断优先级           是
 *  0xB9   SADEN       从机地址掩码(STC)    否
 *  0xC0   XICON       扩展中断(STC)        是
 *  0xC8   T2CON       定时器2控制          是
 *  0xC9   T2MOD       定时器2模式(STC)      否
 *  0xCA   RCAP2L      定时器2捕获低        否
 *  0xCB   RCAP2H      定时器2捕获高        否
 *  0xCC   TL2         定时器2低            否
 *  0xCD   TH2         定时器2高            否
 *  0xD0   PSW         程序状态字           是
 *  0xE0   ACC         累加器               是
 *  0xE1   WDT_CONTR   看门狗(STC扩展)     否
 *  0xE2   ISP_DATA    ISP数据(STC扩展)     否
 *  0xE3   ISP_ADDRH   ISP地址高(STC扩展)   否
 *  0xE4   ISP_ADDRL   ISP地址低(STC扩展)   否
 *  0xE5   ISP_CMD     ISP命令(STC扩展)     否
 *  0xE6   ISP_TRIG    ISP触发(STC扩展)     否
 *  0xE7   ISP_CONTR   ISP控制(STC扩展)     否
 *  0xE8   P4          I/O端口(STC扩展)     是
 *  0xF0   B           B寄存器              是
 *
 *  注: 带括号的地址为 STC 扩展寄存器（标准8051/8052没有）
 *
 * ============================================================ */

#endif /* __STC89C52RC_H__ */
