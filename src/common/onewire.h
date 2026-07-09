/*
 * File:    onewire.h
 * Project: 公共模块 - 1-Wire 软件模拟驱动
 * MCU:     STC89C52RC (11.0592 MHz, 1 机器周期 ≈ 1.085us)
 * Brief:   提供 1-Wire 总线原语: 复位/读/写
 *
 * 1-Wire 时序基于延时循环 (us 级), 不依赖硬件外设
 * 仅支持单从机拓扑或多从机总线仲裁的场景
 *
 * 依赖: 无 (使用 stc89c52rc.h 的 sbit 定义 GPIO)
 */

#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

/* ============================================================
 * 1-Wire 引脚定义 (DQ)
 * ============================================================
 *
 * 1-Wire 总线要求开漏输出, DQ 外部需接 4.7K 上拉电阻到 VCC
 * MCU 内部: 输出低 = 拉低总线; 输出高 = 释放总线 (读外部电平)
 *
 * DQ = P44 (STC89C52RC 扩展端口)
 *
 * 注意: ONEWIRE_DQ 在 onewire.c 中定义为 sbit ONEWIRE_DQ = P4^4;
 * 其他文件通过本头文件使用外部声明。
 */

#ifndef ONEWIRE_C
#define ONEWIRE_DQ  P44
#else
/* onewire.c 内部使用实际 sbit 定义 */
#endif

/* ============================================================
 * API
 * ============================================================ */

/**
 * @brief  初始化 1-Wire 总线 (释放 DQ 置高, 等从机就绪)
 *
 * @code
 *   // 程序入口
 *   OneWire_Init();
 * @endcode
 */
void OneWire_Init(void);

/**
 * @brief  发送复位脉冲并检测从机应答
 * @return 0  - 总线上无应答 (从机不存在)
 *         1  - 检测到从机应答 (存在)
 *
 * @code
 *   if (!OneWire_Reset()) {
 *       // 总线上没有 DS18B20, 提示错误
 *       return;
 *   }
 *   // 总线上有从机, 可以继续发 ROM/Function 命令
 * @endcode
 */
unsigned char OneWire_Reset(void);

/**
 * @brief  向 1-Wire 从机写入 1 字节
 * @param  byte  待写入字节 (低位先发)
 *
 * @code
 *   // 跳过 ROM (单从机时常用)
 *   OneWire_Reset();
 *   OneWire_WriteByte(0xCC);   // SKIP ROM
 *
 *   // 启动温度转换 (DS18B20 Function 命令 0x44)
 *   OneWire_WriteByte(0x44);
 * @endcode
 */
void OneWire_WriteByte(unsigned char byte);

/**
 * @brief  从 1-Wire 从机读取 1 字节
 * @return 读到的字节 (低位先收)
 *
 * @code
 *   unsigned char rom[8];
 *   unsigned char i;
 *
 *   OneWire_Reset();
 *   OneWire_WriteByte(0x33);   // READ ROM
 *   for (i = 0; i < 8; i++) {
 *       rom[i] = OneWire_ReadByte();
 *   }
 *   // rom[0] = 家族码 (DS18B20 = 0x28)
 *   // rom[1..6] = 序列号
 *   // rom[7] = CRC
 * @endcode
 */
unsigned char OneWire_ReadByte(void);

#endif /* __ONEWIRE_H__ */
