# UART 公共模块设计说明

## 1. 概述

UART 模块是项目 `src/common/` 下的串口通信公共库，为 STC89C52RC 提供字节流收发能力，并内置 10ms 软件节拍用于应用层超时判定。

| 项目 | 说明 |
|---|---|
| **文件** | `src/common/uart.h` + `src/common/uart.c` |
| **MCU** | STC89C52RC (11.0592 MHz) |
| **编译工具** | PlatformIO + SDCC 4.4.0 |
| **通信模式** | 模式1, 8N1, 9600 bps (可修改) |

---

## 2. 硬件资源占用

| 资源 | 用途 | 说明 |
|---|---|---|
| **T1** | 波特率发生器 | 模式2 (8 位自动重装), TL1=TH1=0xFD, SMOD=0 |
| **T2** | 10ms 软件 tick | 模式1 (16 位自动重装), 中断向量 5, 供 Uart_GetTick() 和 Uart_GetIdleTicks() |
| **中断 4** | UART 收发 | RI 接收 / TI 发送完成 |
| **中断 5** | T2 节拍 | 每 10ms 触发一次, g_tick 自增 |

**与项目中其他模块的兼容性**：

| 模块 | 占用资源 | 与 UART 共存 |
|---|---|---|
| `timer.h/c` (软件定时调度器) | T0, 中断 1 | ✅ 完全兼容 |
| `smg.h/c` (数码管扫描) | T0, 中断 1 | ✅ 完全兼容 |
| `delay.h/c` | 无定时器 | ✅ 完全兼容 |
| 任何占用 T2 的模块 | T2 | ❌ 冲突 |

---

## 3. 波特率计算

**给定参数**：
- Fosc = 11.0592 MHz
- SMOD = 0 (不倍速)
- T1 模式2 (8 位自动重装), 12 分频

$$\text{BaudRate} = \frac{11,059,200}{12 \times 32 \times (256 - TH1)}$$

当 TH1 = 0xFD (253) 时：

$$\text{BaudRate} = \frac{11,059,200}{384 \times 3} = 9600 \text{ bps}$$

**误差**：理论值与实际值一致，**零误差**。

---

## 4. 模块内部设计

### 4.1 接收环形缓冲

```
        rx_buf[UART_RX_BUF_SIZE]
        +----+----+----+----+-----+----+
        | H  | e  | l  | l  | ... |    |
        +----+----+----+----+-----+----+
         ↑                       ↑
      rx_tail                 rx_head
    (主循环读取)            (ISR 写入)
```

**机制**：
- `rx_head`: 写索引，**仅 UART ISR 写入**，主循环只读
- `rx_tail`: 读索引，**仅主循环写入**
- `(rx_head + 1) % SIZE == rx_tail` 表示缓冲已满（留一格区分满/空）
- 缓冲满时丢弃新字节，不覆盖已存数据

**线程安全**：接收缓冲由中断和主循环并发访问，但由于读写索引分别由不同上下文独占修改，无需关中断保护。

### 4.2 发送机制

- 发送是**阻塞式**：`Uart_SendByte()` 自旋等待 `uart_tx_busy == 0`
- `uart_tx_busy` 由主循环设 1，由 TI 中断设 0
- `__bit` 类型在 8051 上为原子位操作，天然线程安全

```c
void Uart_SendByte(unsigned char ch) {
    while (uart_tx_busy);   // 等上次发送完成
    uart_tx_busy = 1;
    SBUF = ch;              // 硬件启动移位输出
}
```

### 4.3 T2 软件 tick

- T2 配置为 16 位自动重装模式，重载值 65536 - 9216 = 56320
- 11.0592 MHz / 12 / 9216 = 100 Hz → **每 10ms 触发一次中断**
- 全局变量 `g_tick`（`unsigned int`）每 10ms 自增一次
- **首次启动后 `g_tick` 持续递增，不会回卷溢出在 10 小时内（65535×10ms ≈ 655 秒后回卷）**
- `last_rx_tick` 在每次接收到字节时记录当前 tick，由 `Uart_GetIdleTicks()` 返回空闲时长

**T2 初始化代码**：

```c
T2CON = 0;                          // 清控制位, 默认定时模式
RCAP2L = (unsigned char)(56320 & 0xFF);
RCAP2H = (unsigned char)(56320 >> 8);
TL2 = RCAP2L;
TH2 = RCAP2H;
TR2 = 1;                            // 启动 T2
ET2 = 1;                            // 允许 T2 中断
```

### 4.4 `Uart_GetTick()` 的原子性保证

在 8 位 MCU 上读取 16 位变量可能被中断切割，导致读到错误值。采用**双次读取 + 比对**策略：

```c
unsigned int Uart_GetTick(void) {
    unsigned int a, b;
    do {
        a = g_tick;
        b = g_tick;
    } while (a != b);
    return a;
}
```

---

## 5. API 完整参考

### 5.1 初始化

| 函数 | 说明 |
|---|---|
| `void Uart_Init(void)` | 初始化 UART 硬件 (T1 波特率 + T2 tick + 串口中断) |

**调用时机**：在 `main()` 开头，`EA=1` 开启后初始化。

### 5.2 发送

| 函数 | 说明 | 阻塞 |
|---|---|---|
| `void Uart_SendByte(unsigned char ch)` | 发送单个字节 | 是（等忙标志） |
| `void Uart_SendStr(const char *s)` | 发送 `\0` 结尾字符串 | 是 |
| `void Uart_SendBytes(const unsigned char *data, unsigned char len)` | 发送指定长度二进制 | 是 |

### 5.3 接收

| 函数 | 说明 |
|---|---|
| `unsigned char Uart_Available(void)` | 接收缓冲是否有未读数据 |
| `unsigned char Uart_ReadByte(unsigned char *ch)` | 读一个字节，返回 1=成功 0=空 |
| `unsigned char Uart_Peek(unsigned char *ch)` | 窥视最早字节（不移除） |
| `void Uart_FlushRx(void)` | 清空接收缓冲 |
| `unsigned char Uart_RxCount(void)` | 返回当前未读字节数 |

### 5.4 时间戳

| 函数 | 说明 |
|---|---|
| `unsigned int Uart_GetTick(void)` | 返回自 Uart_Init 以来的总 tick 数（每 10ms 增 1） |
| `unsigned int Uart_GetIdleTicks(void)` | 返回自上次接收到字节以来的空闲 tick 数 |

---

## 6. 使用示例

### 6.1 最简回显程序

```c
#include <stc89c52rc.h>
#include "uart.h"

void main(void) {
    unsigned char ch;

    Uart_Init();
    Uart_SendStr("Echo mode\r\n");

    while (1) {
        if (Uart_Available()) {
            Uart_ReadByte(&ch);
            Uart_SendByte(ch);           // 原样回显
        }
    }
}
```

### 6.2 命令行程序（超时判定）

```c
#include <stc89c52rc.h>
#include "uart.h"

#define BUF_SIZE  24
#define TIMEOUT   30   // 30 × 10ms = 300ms

static unsigned char buf[BUF_SIZE];
static unsigned char len;

void main(void) {
    unsigned char ch;

    Uart_Init();
    Uart_SendStr("> ");

    while (1) {
        while (Uart_Available()) {
            Uart_ReadByte(&ch);
            if (ch == '\r' || ch == '\n') {
                Uart_SendStr("\r\n");
                buf[len] = '\0';
                Uart_SendStr("You typed: ");
                Uart_SendStr((const char *)buf);
                Uart_SendStr("\r\n> ");
                len = 0;
            } else if (len < BUF_SIZE - 1) {
                buf[len++] = ch;
                Uart_SendByte(ch);
            }
        }

        /* 超时处理 */
        if (len > 0 && Uart_GetIdleTicks() >= TIMEOUT) {
            Uart_SendStr("\r\n[timeout]\r\n> ");
            len = 0;
        }
    }
}
```

### 6.3 项目中引用 UART 模块

在 `platformio.ini` 中为子项目添加：

```ini
[env:my-project]
extends = common
build_src_filter = +<my-project/*> +<common/uart.c>
build_flags =
    ${common.build_flags}
    --opt-code-size
    -Isrc/my-project
    -Isrc/common
```

**重要**：
- 只需引用 `uart.c`（`uart.h` 的路径由 `-Isrc/common` 包含）
- 不需要引用整个 `common/*` 目录（会引入不必要的 timer/smg/matrix 等，造成 IRAM 不足）

---

## 7. 配置项

在 `#include "uart.h"` **之前**可通过宏覆盖默认值：

```c
#define UART_RX_BUF_SIZE  64   // 增大接收缓冲（默认 32 字节）
#include "uart.h"
```

| 宏 | 默认值 | 说明 |
|---|---|---|
| `UART_RX_BUF_SIZE` | 32 | 接收环形缓冲字节数 |
| `UART_TICK_MS` | 10 | tick 周期（ms），由 T2 定时器硬件决定，不建议修改 |

**修改波特率**需直接修改 [uart.c](file:///c:/workspace/51-scm/src/common/uart.c) 第 73-74 行的 `TH1/TL1` 值。

---

## 8. 资源占用

| 指标 | 数值 |
|---|---|
| Flash | 约 480 字节（仅 uart.o） |
| IRAM | rx_buf[32] + 全局变量 ≈ 40 字节 |
| 定时器 | T1, T2 |
| 中断向量 | 4 (UART), 5 (T2) |

---

## 9. 注意事项

1. **Uart_Init() 执行后 `EA=1`**，此后所有中断处于开启状态，需确保其他外设初始化在之前完成
2. **发送是阻塞式**，不要在主循环中等待发送完成时进行关键实时操作
3. **接收缓冲满时丢弃新字节**（不覆盖），如果需绝对不丢数据，应增大 `UART_RX_BUF_SIZE`
4. **T2 被永久占用**，不能与使用 T2 的模块同时使用
5. **`Uart_GetIdleTicks()` 在 ISR 中有微小延迟**（T2 tick 更新比 RI 触发晚 0-10ms），不影响超时判定
6. **若需在项目中使用 timer 模块的任务调度器**，T0 完全独立，无冲突
