/* ============================================================================
 * File:    main.c
 * Project: 06-uart-pc
 * MCU:     STC89C52RC (11.0592 MHz)
 * Tool:    PlatformIO + SDCC
 *
 * 行为(命令行风格协议):
 *   - 上电后通过公共 UART 模块发送 "Hello, World!\r\n"
 *   - 接收任意字节, 边收边回显
 *   - 收到 '\r' 或 '\n' 立即处理当前缓冲作为一条命令
 *   - 否则 300ms 无新字符视为命令结束
 *   - 命令不区分大小写
 *
 *   支持命令:
 *     on              点亮 LED
 *     off             熄灭 LED
 *     hello           回应 "Hi!"
 *     echo <text>     回显 <text>
 *     help            列出可用命令
 *
 *   任何未识别命令 → 回应 "ERR: unknown command\r\n"
 *
 * 依赖: src/common/uart.h, src/common/uart.c
 * ========================================================================== */

#include <stc89c52rc.h>
#include "uart.h"

#define LED_PIN             P00                              // LED 接到 P0.0
#define LINE_BUF_SIZE       24                              // 单条命令最大长度(不含'\0')
#define RX_IDLE_TICKS       30                              // 300ms(每 tick 10ms, 见 uart.h)

/* —— 命令行编辑缓冲 —— */                            // 段: 命令行缓冲
static unsigned char line_buf[LINE_BUF_SIZE];          // 当前正在编辑的命令
static unsigned char line_len;                         // 已编辑字符数
/* 注: 接收时间戳由 UART 模块维护, 通过 Uart_GetIdleTicks() 读取 */

/* —— 字符工具 —— */                                  // 段: 字符处理小工具
static unsigned char IsSpace(unsigned char c)          // 判断是否为空白字符
{
    return (c == ' ' || c == '\t');                    // 空格或制表符
}

static unsigned char ToLower(unsigned char c)          // ASCII 字符转小写
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c; // A-Z 范围内偏移, 其它字符原样返回
}

// 前缀比较(忽略大小写), 比较 n 字节; 返回 1 表示 line 前 n 字节匹配 pattern
static unsigned char PrefixEq(const char *line, const char *pattern, unsigned char n)
{
    while (n--) {                                      // 逐字节比较 n 次
        if (ToLower((unsigned char)*line) != ToLower((unsigned char)*pattern)) { // 不相等
            return 0;                                  // 返回 0 表示不等
        }
        line++;                                        // 指针前进
        pattern++;                                     // 模板指针前进
    }
    return 1;                                          // n 字节全部相等, 返回 1
}

/* —— 命令处理 —— */                                  // 段: 命令解析与分发
static void Cmd_Handle(unsigned char len)              // 解析并执行一条命令
{
    unsigned char *line = line_buf;                    // 指向本地缓冲
    unsigned char i;                                   // 循环变量

    // 跳过行首空白(原地左移)
    while (len > 0 && IsSpace(line[0])) {              // 首字符是空白就左移
        unsigned char k;                               // 内层循环索引
        for (k = 1; k < len; k++) {                    // 从第 2 个字符起全部前移 1 位
            line[k - 1] = line[k];                     // 覆盖前一个位置
        }
        len--;                                         // 有效长度减 1
    }

    if (len == 0) {                                    // 全部是空白
        return;                                        // 空行不响应
    }

    line[len] = '\0';                                  // 末尾补 '\0' 形成 C 字符串

    if (PrefixEq(line, "on", 2) && (len == 2 || IsSpace(line[2]))) { // 匹配 "on"
        LED_PIN = 0;                                   // 点亮 LED
        Uart_SendStr("OK: LED ON\r\n");                // 回执成功
        return;                                        // 处理完返回
    }

    if (PrefixEq(line, "off", 3) && (len == 3 || IsSpace(line[3]))) { // 匹配 "off"
        LED_PIN = 1;                                   // 熄灭 LED
        Uart_SendStr("OK: LED OFF\r\n");               // 回执成功
        return;                                        // 处理完返回
    }

    if (PrefixEq(line, "hello", 5) && (len == 5 || IsSpace(line[5]))) { // 匹配 "hello"
        Uart_SendStr("Hi!\r\n");                       // 回执问候
        return;                                        // 处理完返回
    }

    if (PrefixEq(line, "help", 4) && (len == 4 || IsSpace(line[4]))) { // 匹配 "help"
        Uart_SendStr("Commands:\r\n");                 // 帮助标题
        Uart_SendStr("  on            LED ON\r\n");    // on 说明
        Uart_SendStr("  off           LED OFF\r\n");   // off 说明
        Uart_SendStr("  hello         reply Hi!\r\n"); // hello 说明
        Uart_SendStr("  echo <text>   echo text\r\n"); // echo 说明
        Uart_SendStr("  help          list commands\r\n"); // help 说明
        return;                                        // 处理完返回
    }

    if (PrefixEq(line, "echo", 4) && IsSpace(line[4])) { // 匹配 "echo <msg>"
        unsigned char k = 5;                           // 跳过 "echo " 这 5 个字符
        while (k < len && IsSpace(line[k])) k++;       // 跳过 echo 后面的多余空白
        Uart_SendStr((const char *)(line + k));        // 输出剩余内容
        Uart_SendStr("\r\n");                          // 补上换行
        return;                                        // 处理完返回
    }

    // unknown
    Uart_SendStr("ERR: unknown command '");            // 错误前缀
    for (i = 0; i < len && i < LINE_BUF_SIZE; i++) {   // 逐字节回显原命令
        Uart_SendByte(line[i]);                        // 输出单个字符
    }
    Uart_SendStr("'\r\n");                             // 错误后缀
}

/* —— 主循环 —— */                                    // 段: 主函数
void main(void)                                        // 程序入口
{
    unsigned char ch;                                  // 接收字节暂存

    LED_PIN = 1;                                       // 熄灭 LED
    P0 = 0xFF;                                         // P0 端口全部置高, 防止干扰

    line_len = 0;                                      // 命令缓冲清空

    Uart_Init();                                       // 初始化串口, 启动 T1 波特率 + T2 tick

    Uart_SendStr("Hello, World!\r\n");                 // 发送启动问候语

    while (1) {                                        // 主循环, 永不退出
        /* —— 1) 处理所有待读接收字节 —— */           // 段: 接收处理
        while (Uart_Available()) {                     // 接收缓冲非空
            (void)Uart_ReadByte(&ch);                  // 读取一个字节

            if (ch == '\b' || ch == 0x7F) {            // 退格或 DEL 键
                if (line_len > 0) {                    // 缓冲非空
                    line_len--;                        // 长度减 1
                    Uart_SendByte('\b');               // 光标回退
                    Uart_SendByte(' ');                // 空格覆盖
                    Uart_SendByte('\b');               // 再次回退
                }
            } else if (ch == '\r' || ch == '\n') {     // 回车或换行
                Uart_SendStr("\r\n");                  // 主机端输出换行
                if (line_len > 0) {                    // 有内容才解析
                    Cmd_Handle(line_len);              // 处理这条命令
                }
                line_len = 0;                          // 清空命令缓冲
            } else {                                   // 普通可显示字符
                if (line_len < LINE_BUF_SIZE) {        // 缓冲未满
                    line_buf[line_len++] = ch;         // 入缓冲
                    Uart_SendByte(ch);                 // 回显字符
                } else {                               // 缓冲已满
                    Uart_SendByte('\a');               // 输出响铃
                }
            }
            /* UART 模块内部已维护接收时间戳, 应用层无需刷新 */
        }

        /* —— 2) 检测超时 —— */                       // 段: 超时判定
        if (line_len > 0) {                            // 有未结束的输入
            unsigned char diff = (unsigned char)(Uart_GetIdleTicks()); // 计算空闲时长
            if (diff >= RX_IDLE_TICKS) {               // 超过 300ms
                Cmd_Handle(line_len);                  // 解析并执行
                line_len = 0;                          // 清空缓冲
            }
        }
    }
}