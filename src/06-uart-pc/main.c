/* ============================================================================
 * File:    main.c
 * Project: 06-uart-pc
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 *
 * 行为(命令行风格协议):
 *   - 上电后串口发送一次 "Hello, World!\r\n"
 *   - 串口接收任意字节, 边收边回显(便于在主机看到输入)
 *   - 300ms 内未收到新字符 → 视为一条命令结束, 解析并执行
 *   - 命令不区分大小写, 命令与参数以空格分隔
 *
 *   支持命令:
 *     on              点亮 LED
 *     off             熄灭 LED
 *     hello           回应 "Hi!"
 *     echo <text>     回显 <text>
 *     help            列出可用命令
 *
 *   任何未识别命令 → 回应 "ERR: unknown command\r\n"
 * ========================================================================== */

#include <stc89c52rc.h>                                               // MCU 特殊功能寄存器声明

#define LED_PIN             P00                                      // LED 接到 P0.0
#define UART_RX_BUF_SIZE    24                                       // 单条命令最大长度(不含'\0')
#define RX_LINE_TIMEOUT_MS  30                                       // g_tick 每 10ms 增 1, 30 对应 300ms
#define TIMER0_RELOAD_10MS  (65536 - 9216)                            // 11.0592MHz / 12 / 100 = 9216, 16 位定时器初值

/* —— 发送相关 —— */                                                // 段: 串口发送状态
static volatile __bit uart_tx_busy = 0;                               // 发送忙标志, 1=有字节正在移位输出

/* —— 接收相关(线性缓冲) —— */                                      // 段: 串口接收缓冲
static volatile unsigned char rx_buf[UART_RX_BUF_SIZE];              // 接收字节线性缓冲
static volatile unsigned char rx_len;                                // 当前已接收字节数
static volatile unsigned char rx_last_tick;                           // 上次收到字节时的 g_tick 值(用于超时判定)

/* —— 10ms 节拍 —— */                                                // 段: 软件定时器
static volatile unsigned char g_tick;                                // 由 Timer0 每 10ms 自增一次的计数器

/* —— 串口底层 —— */                                                // 段: 串口收发函数
static void Uart_SendByte(unsigned char ch)                          // 发送单个字节(阻塞)
{
    while (uart_tx_busy) {                                           // 自旋等待上一次发送完成(TI 中断会清标志)
    }
    uart_tx_busy = 1;                                                // 抢占忙标志
    SBUF = ch;                                                       // 写入发送缓冲, 硬件启动移位输出
}

static void Uart_SendStr(const char *s)                              // 发送以 '\0' 结尾的字符串
{
    while (*s) {                                                     // 遍历到字符串末尾
        Uart_SendByte((unsigned char)*s++);                          // 逐字节发送
    }
}

/* —— 字符工具 —— */                                                // 段: 字符处理小工具
static unsigned char IsSpace(unsigned char c)                        // 判断是否为空白字符
{
    return (c == ' ' || c == '\t');                                  // 空格或制表符
}

static unsigned char ToLower(unsigned char c)                        // ASCII 字符转小写
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;            // A-Z 范围内偏移, 其它字符原样返回
}

// 前缀比较(忽略大小写), 比较 n 字节; 返回 1 表示 line 前 n 字节匹配 pattern
static unsigned char PrefixEq(const char *line, const char *pattern, unsigned char n)
{
    while (n--) {                                                    // 逐字节比较 n 次
        if (ToLower((unsigned char)*line) != ToLower((unsigned char)*pattern)) { // 不相等
            return 0;                                                // 返回 0 表示不等
        }
        line++;                                                      // 指针前进
        pattern++;                                                   // 模板指针前进
    }
    return 1;                                                        // n 字节全部相等, 返回 1
}

/* —— 命令处理 —— */                                                // 段: 命令解析与分发
static void Cmd_Handle(unsigned char len)                            // 解析并执行一条命令, len 为命令字节数
{
    unsigned char *line = (unsigned char *)rx_buf;                   // 直接在接收缓冲上原地处理(节省栈)
    unsigned char i;                                                 // 循环变量

    // 跳过行首空白(直接在原缓冲上操作, 不复制)
    while (len > 0 && IsSpace(line[0])) {                            // 首字符是空白就左移
        unsigned char k;                                             // 内层循环索引
        for (k = 1; k < len; k++) {                                  // 从第 2 个字符起全部前移 1 位
            line[k - 1] = line[k];                                   // 覆盖前一个位置
        }
        len--;                                                       // 有效长度减 1
    }

    if (len == 0) {                                                  // 全部是空白
        return;                                                      // 空行不响应
    }

    line[len] = '\0';                                                // 末尾补 '\0' 形成 C 字符串

    if (PrefixEq(line, "on", 2) && (len == 2 || IsSpace(line[2]))) { // 匹配 "on" 单独成词
        LED_PIN = 0;                                                 // 点亮 LED(低电平点亮)
        Uart_SendStr("OK: LED ON\r\n");                              // 回执成功
        return;                                                      // 处理完返回
    }

    if (PrefixEq(line, "off", 3) && (len == 3 || IsSpace(line[3]))) { // 匹配 "off" 单独成词
        LED_PIN = 1;                                                 // 熄灭 LED(高电平熄灭)
        Uart_SendStr("OK: LED OFF\r\n");                             // 回执成功
        return;                                                      // 处理完返回
    }

    if (PrefixEq(line, "hello", 5) && (len == 5 || IsSpace(line[5]))) { // 匹配 "hello"
        Uart_SendStr("Hi!\r\n");                                     // 回执问候
        return;                                                      // 处理完返回
    }

    if (PrefixEq(line, "help", 4) && (len == 4 || IsSpace(line[4]))) { // 匹配 "help"
        Uart_SendStr("Commands:\r\n");                               // 帮助标题
        Uart_SendStr("  on            LED ON\r\n");                  // on 说明
        Uart_SendStr("  off           LED OFF\r\n");                 // off 说明
        Uart_SendStr("  hello         reply Hi!\r\n");               // hello 说明
        Uart_SendStr("  echo <text>   echo text\r\n");               // echo 说明
        Uart_SendStr("  help          list commands\r\n");           // help 说明
        return;                                                      // 处理完返回
    }

    if (PrefixEq(line, "echo", 4) && IsSpace(line[4])) {             // 匹配 "echo <msg>"
        unsigned char k = 5;                                         // 跳过 "echo " 这 5 个字符
        while (k < len && IsSpace(line[k])) k++;                     // 跳过 echo 后面的多余空白
        Uart_SendStr((const char *)(line + k));                      // 输出剩余内容
        Uart_SendStr("\r\n");                                        // 补上换行
        return;                                                      // 处理完返回
    }

    // unknown
    Uart_SendStr("ERR: unknown command '");                          // 错误前缀
    for (i = 0; i < len && i < UART_RX_BUF_SIZE - 1; i++) {         // 逐字节回显原命令
        Uart_SendByte(line[i]);                                      // 输出单个字符
    }
    Uart_SendStr("'\r\n");                                           // 错误后缀
}

/* —— 串口初始化 —— */                                              // 段: 串口硬件配置
static void Uart_Init(void)                                          // 配置串口模式与中断
{
    SM0 = 0;                                                         // 串口模式位 0
    SM1 = 1;                                                         // 串口模式位 1, 组合=模式1(8N1)
    PCON &= 0x7F;                                                    // 清 SMOD 位, 不倍速, 波特率精确

    TMOD &= 0x0F;                                                    // 保留 T0 设置, 清 T1 模式位
    TMOD |= 0x20;                                                    // T1 设置为模式2(8 位自动重装)
    TL1 = 0xFD;                                                      // T1 低字节初值 253
    TH1 = 0xFD;                                                      // T1 重装值 253, 11.0592MHz 下产生 9600bps
    TR1 = 1;                                                         // 启动 T1 作为波特率发生器

    REN = 1;                                                         // 允许串口接收
    SM2 = 0;                                                         // 模式1 下不进行多机通信校验
    RI = 0;                                                          // 清接收中断标志
    TI = 0;                                                          // 清发送中断标志

    EA = 1;                                                          // 开放全局中断
    ES = 1;                                                          // 开放串口中断
}

/* —— Timer0: 10ms 节拍 —— */                                        // 段: 10ms 节拍定时器配置
static void Timer0_Init(void)                                        // 配置 T0 为 16 位定时
{
    TMOD &= 0xF0;                                                    // 保留 T1 设置, 清 T0 模式位
    TMOD |= 0x01;                                                    // T0 设置为模式1(16 位定时)
    TH0 = (unsigned char)(TIMER0_RELOAD_10MS >> 8);                  // T0 高字节初值
    TL0 = (unsigned char)(TIMER0_RELOAD_10MS & 0xFF);                // T0 低字节初值
    TR0 = 1;                                                         // 启动 T0
    ET0 = 1;                                                         // 允许 T0 中断
}

/* —— 中断服务 —— */                                                // 段: 中断向量
void Uart_Isr(void) __interrupt(4)                                   // 串口中断入口, 向量号 4
{
    unsigned char c;                                                 // 接收字节暂存

    if (RI) {                                                        // 接收完成中断
        RI = 0;                                                      // 软件清接收标志
        c = SBUF;                                                    // 读取接收字节

        if (c == '\b' || c == 0x7F) {                                // 退格或 DEL 键
            // 退格: 弹出一个字符, 终端侧回退
            if (rx_len > 0) {                                        // 缓冲非空
                rx_len--;                                            // 长度减 1
                Uart_SendByte('\b');                                 // 光标回退一格
                Uart_SendByte(' ');                                  // 输出空格覆盖原字符
                Uart_SendByte('\b');                                 // 光标再次回退
            }
        } else if (c == '\r' || c == '\n') {                         // 回车或换行
            // 换行: 立即结束当前命令(不必等超时)
            Uart_SendStr("\r\n");                                    // 主机端输出换行
            if (rx_len > 0) {                                        // 缓冲非空才解析
                Cmd_Handle(rx_len);                                  // 处理这条命令
            }
            rx_len = 0;                                              // 清空接收长度
            rx_last_tick = g_tick;                                   // 刷新时间戳, 重新开始计时
            return;                                                  // 已处理 RI, 跳过最后的时间戳统一更新
        } else {                                                     // 普通可显示字符
            // 普通字符: 入缓冲并回显
            if (rx_len < UART_RX_BUF_SIZE - 1) {                     // 缓冲未满
                rx_buf[rx_len++] = c;                                // 入缓冲, 长度加 1
                Uart_SendByte(c);                                    // 回显字符
            } else {                                                 // 缓冲已满
                Uart_SendByte('\a');                                 // 输出响铃提示用户
            }
        }

        rx_last_tick = g_tick;                                       // 任何输入都刷新时间戳, 重置超时
    }

    if (TI) {                                                        // 发送完成中断
        TI = 0;                                                      // 软件清发送标志
        uart_tx_busy = 0;                                            // 释放发送忙标志, 唤醒等待者
    }
}

void Timer0_Isr(void) __interrupt(1)                                 // T0 中断入口, 向量号 1
{
    TH0 = (unsigned char)(TIMER0_RELOAD_10MS >> 8);                  // 重装 T0 高字节
    TL0 = (unsigned char)(TIMER0_RELOAD_10MS & 0xFF);                // 重装 T0 低字节
    g_tick++;                                                        // 全局 tick 计数器加 1(每 10ms 一次)
}


/* —— 主循环 —— */                                                  // 段: 主函数
void main(void)                                                      // 程序入口
{
    LED_PIN = 1;                                                     // 熄灭 LED(高电平)
    P0 = 0xFF;                                                       // P0 端口全部置高, 防止干扰

    rx_len = 0;                                                      // 接收长度清零
    rx_last_tick = 0;                                                // 时间戳清零
    g_tick = 0;                                                      // 全局 tick 清零

    Timer0_Init();                                                   // 初始化 T0 提供节拍
    Uart_Init();                                                     // 初始化串口

    Uart_SendStr("Hello, World!\r\n");                               // 发送启动问候语

    while (1) {                                                      // 主循环, 永不退出
        if (rx_len > 0) {                                            // 接收缓冲非空, 可能是一行命令
            unsigned char diff = (unsigned char)(g_tick - rx_last_tick); // 计算空闲时长(tick 数)
            if (diff >= RX_LINE_TIMEOUT_MS) {                        // 超过 300ms 未收到新字节
                Cmd_Handle(rx_len);                                  // 解析并执行命令
                rx_len = 0;                                          // 清空接收长度, 准备下一条
            }
        }
    }
}