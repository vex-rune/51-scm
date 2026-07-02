# VSCode 开发 51 单片机完整指南

## 一、内容

- VSCode 本身**不包含编译器**，需要搭配外部工具链
- 主流方案：**VSCode 编辑代码 + SDCC 编译 + STC-ISP 烧录**
- 关键插件：**Embedded IDE** 或 **PlatformIO IDE**
- 烧录方式：可通过命令行工具或插件集成实现

## 二、要点说明

### 2.1 VSCode 与 Keil 的对比

| 对比项 | Keil | VSCode |
| :--- | :--- | :--- |
| 性质 | IDE（集成开发环境） | 代码编辑器 |
| 自带编译器 | ✅ Keil C51 Compiler | ❌ 需自己装 |
| 烧录工具 | ❌ 需外接 STC-ISP | ❌ 需外接 |
| 编辑体验 | 一般 | ⭐ 优秀 |
| 插件生态 | 弱 | ⭐ 丰富 |
| 体积 | 较大 | 轻量 |
| 调试功能 | ⭐ 完善 | 较弱 |
| 上手成本 | 低 | 较高 |

### 2.2 三种 VSCode + 51 单片机方案

| 方案 | 编辑器 | 编译器 | 烧录 | 难度 |
| :--- | :--- | :--- | :--- | :--- |
| **方案一** | VSCode | Keil C51（命令行调用） | STC-ISP | ⭐⭐ |
| **方案二** | VSCode | SDCC（开源编译器） | stcgal（命令行烧录） | ⭐⭐⭐ |
| **方案三** | VSCode + Embedded IDE 插件 | 插件内置 | 插件集成 | ⭐⭐ |

### 2.3 方案一：VSCode + Keil C51（推荐）

#### 思路

VSCode 只负责**写代码**，编译和烧录继续用 **Keil** + **STC-ISP**。

#### 配置步骤

| 步骤 | 操作 |
| :--- | :--- |
| 1 | 安装 VSCode |
| 2 | 安装 Keil C51（提供 C51 编译器） |
| 3 | VSCode 安装插件：`C/C++`、`Keil Assistant`（可选） |
| 4 | 在 Keil 中创建工程并配置好 |
| 5 | VSCode 打开 Keil 工程目录编辑代码 |
| 6 | 回到 Keil 编译 |
| 7 | 用 STC-ISP 烧录生成的 .hex 文件 |

#### 推荐插件

| 插件名 | 功能 |
| :--- | :--- |
| **C/C++** | 代码高亮、智能提示 |
| **C51 语法高亮** | 51 单片机 C 语法识别 |
| **Keil Assistant** | 在 VSCode 中直接调用 Keil 编译 |

### 2.4 方案二：VSCode + SDCC（开源方案）

#### 思路

完全脱离 Keil，用开源工具链。

#### 工具清单

| 工具 | 作用 |
| :--- | :--- |
| **VSCode** | 代码编辑 |
| **SDCC** | Small Device C Compiler，开源 51 编译器 |
| **stcgal** | STC 单片机开源烧录工具（Python） |
| **make** | 构建工具 |

#### 安装步骤

```bash
# 1. 安装 SDCC（Windows）
# 下载地址：http://sdcc.sourceforge.net/
# 或使用 choco: choco install sdcc

# 2. 安装 Python 和 stcgal
pip install stcgal

# 3. 安装 VSCode 插件
# - C/C++
# - Embedded IDE（推荐）
```

#### 编译命令

```bash
sdcc -o build main.c delay.c
# 生成 build/ihx 文件
```

#### 烧录命令

```bash
stcgal -p COM3 -b 115200 build/main.ihx
```

> **注意**：stcgal 只支持 .ihx 文件，需要先用 `sdcc-objcopy` 转换或调整参数。

### 2.5 方案三：VSCode + Embedded IDE 插件（最推荐）

#### 简介

**Embedded IDE** 插件专门为单片机开发设计，可一站式完成编辑、编译、烧录。

#### 主要功能

| 功能 | 说明 |
| :--- | :--- |
| 代码编辑 | C/C++ 语法高亮、智能补全 |
| 工程管理 | 支持 Keil、SDCC、IAR 等工程 |
| 编译集成 | 一键调用 SDCC 或 Keil C51 |
| 烧录集成 | 支持 stcgal、STC-ISP 等 |
| 调试功能 | 部分支持 |

#### 使用步骤

| 步骤 | 操作 |
| :--- | :--- |
| 1 | VSCode 搜索安装 `Embedded IDE` 插件 |
| 2 | 工具链选择 SDCC |
| 3 | 创建 51 单片机工程 |
| 4 | 编写代码 |
| 5 | 点击"编译"按钮生成 .hex |
| 6 | 点击"烧录"按钮写入单片机 |

### 2.6 烧录 51 单片机的两种方式

| 方式 | 工具 | 操作 |
| :--- | :--- | :--- |
| **图形界面** | STC-ISP | 选择 .hex 文件 → 选择串口 → 点击下载 |
| **命令行** | stcgal | `stcgal -p COM3 file.hex` |

#### STC-ISP 烧录步骤

| 步骤 | 操作 |
| :--- | :--- |
| 1 | USB 转串口线连接单片机到电脑 |
| 2 | 打开 STC-ISP |
| 3 | 选择单片机型号：STC89C52RC |
| 4 | 选择 COM 端口 |
| 5 | 选择编译生成的 .hex 文件 |
| 6 | 设置波特率（默认 115200） |
| 7 | 点击"下载/编程" |
| 8 | 给单片机上电或按复位键 |
| 9 | 烧录完成 |

## 三、推荐方案

### 3.1 不同人群推荐

| 人群 | 推荐方案 | 理由 |
| :--- | :--- | :--- |
| **初学者** | Keil + STC-ISP | 资料多、配置简单 |
| **VSCode 爱好者** | VSCode + Keil C51 + STC-ISP | 编辑体验好，编译稳定 |
| **完全开源控** | VSCode + SDCC + stcgal | 无版权问题 |
| **追求一站式** | VSCode + Embedded IDE | 全在 VSCode 内完成 |

### 3.2 最终推荐配置 ⭐

```
┌─────────────────────────────────────┐
│  编辑：VSCode + C/C++ 插件            │
│  编译：Keil C51（命令行调用）         │
│  烧录：STC-ISP（图形界面）            │
└─────────────────────────────────────┘
```

> 这种方案既享受了 VSCode 的优秀编辑体验，又保持了 Keil 编译的稳定性，烧录继续用 STC-ISP，最稳妥。

## 四、心得

> 51 单片机开发**不一定非要用 Keil**。如果你更习惯 VSCode：
> 1. **最稳妥**：VSCode 写代码 + Keil 编译 + STC-ISP 烧录
> 2. **完全开源**：VSCode + SDCC + stcgal（命令行烧录）
> 3. **完全图形化**：VSCode + Embedded IDE 插件（一站式）
>
> **烧录的本质**：把 .hex 文件通过**串口**发送到单片机，与用什么编辑器无关。所以无论哪种方案，最终烧录步骤都一样：**连接串口 → 选择 .hex → 触发下载**。

## 五、下一步

- [ ] 安装 Keil C51 + 注册机
- [ ] 安装 VSCode 必备插件（C/C++、Embedded IDE）
- [ ] 安装 STC-ISP 烧录工具
- [ ] 实战：创建第一个 VSCode + Keil 联合工程
- [ ] 实战：烧录 LED 闪烁程序到 STC89C52RC