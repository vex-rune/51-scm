/*
 * File:    main.c
 * Project: 08-eeprom
 * MCU:     STC89C52RC
 * Brief:   串口控制 EEPROM 5 种读写方式
 *
 * 串口命令协议（9600, 8N1）：
 *   R<addr>             Random Read - 读指定地址一个字节
 *   W<addr>,<data>      Byte Write - 写一个字节
 *   P<addr>,<n>,d1,d2.. Page Write - 页写 (最多 16 字节)
 *   C<addr>             Current Address Read - 读当前地址
 *   S<addr>,<n>         Sequential Read - 连续读 N 字节
 *   T                   测试 5 种方式
 *   ?                   帮助
 */
#include <stc89c52rc.h>
#include <delay.h>
#include "eeprom.h"  /* 直接调用 EEPROM_ByteWrite / EEPROM_RandomRead 等接口, 无需向后兼容宏 */
#include "uart.h"
#include "log.h"

#define LED P00

#define CMD_BUF_SIZE 16
static unsigned char cmd_buf[CMD_BUF_SIZE];
static unsigned char cmd_len = 0;

/* 十六进制字符转数值 */
static unsigned char hex2byte(const unsigned char *s)
{
    unsigned char hi = 0, lo = 0;
    if (s[0] >= '0' && s[0] <= '9') hi = s[0] - '0';
    else if (s[0] >= 'A' && s[0] <= 'F') hi = s[0] - 'A' + 10;
    else if (s[0] >= 'a' && s[0] <= 'f') hi = s[0] - 'a' + 10;
    if (s[1] >= '0' && s[1] <= '9') lo = s[1] - '0';
    else if (s[1] >= 'A' && s[1] <= 'F') lo = s[1] - 'A' + 10;
    else if (s[1] >= 'a' && s[1] <= 'f') lo = s[1] - 'a' + 10;
    return (unsigned char)((hi << 4) | lo);
}

static void byte2hex(unsigned char b, char *out)
{
    const char hex[] = "0123456789ABCDEF";
    out[0] = hex[(b >> 4) & 0x0F];
    out[1] = hex[b & 0x0F];
    out[2] = '\0';
}

/* 解析一个十六进制字节, 返回指向下一个分隔符位置 */
static unsigned char parse_hex_byte(const unsigned char *s, unsigned char *end)
{
    unsigned char val;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    val = hex2byte(s);
    s += 2;
    *end = (unsigned char)(s - cmd_buf);
    return val;
}

/* 跳到下一个 ',' 之后的位置 */
static unsigned char skip_to_comma(unsigned char i)
{
    while (i < cmd_len && cmd_buf[i] != ',') i++;
    if (i < cmd_len && cmd_buf[i] == ',') i++;
    return i;
}

static void process_cmd(void)
{
    unsigned char i, addr, data, n, end;
    char hex[4];

    i = 0;
    while (i < cmd_len && (cmd_buf[i] == ' ' || cmd_buf[i] == '\r')) i++;
    if (i >= cmd_len) return;

    switch (cmd_buf[i]) {
    case '?':
        Uart_SendStr("Commands:\n");
        Uart_SendStr("  R<hex>            Random Read\n");
        Uart_SendStr("  W<hex>,<hex>      Byte Write\n");
        Uart_SendStr("  P<hex>,<n>,d...   Page Write (n<=16)\n");
        Uart_SendStr("  C                 Current Address Read\n");
        Uart_SendStr("  S<hex>,<dec>      Sequential Read\n");
        Uart_SendStr("  T                 Test all 5 methods\n");
        break;

    case 'R':   /* Random Read */
    case 'r':
        i++;
        addr = parse_hex_byte(&cmd_buf[i], &end);
        data = EEPROM_RandomRead(addr);
        Uart_SendStr("OK 0x");
        byte2hex(data, hex);
        Uart_SendStr(hex);
        Uart_SendStr("\n");
        break;

    case 'W':   /* Byte Write */
    case 'w':
        i++;
        addr = parse_hex_byte(&cmd_buf[i], &end);
        i = end;
        i = skip_to_comma(i);
        data = parse_hex_byte(&cmd_buf[i], &end);
        EEPROM_ByteWrite(addr, data);
        Uart_SendStr("OK\n");
        LED = 0; DelayMs(50); LED = 1;
        break;

    case 'P': { /* Page Write */
        unsigned char buf[16];
        i++;
        addr = parse_hex_byte(&cmd_buf[i], &end);
        i = end;
        i = skip_to_comma(i);
        /* 读取十进制 n */
        n = 0;
        while (i < cmd_len && cmd_buf[i] >= '0' && cmd_buf[i] <= '9') {
            n = (unsigned char)(n * 10 + (cmd_buf[i] - '0'));
            i++;
        }
        /* 读取 n 个十六进制数据 */
        for (data = 0; data < n && data < 16; data++) {
            i = skip_to_comma(i);
            buf[data] = parse_hex_byte(&cmd_buf[i], &end);
            i = end;
        }
        EEPROM_PageWrite(addr, buf, n);
        Uart_SendStr("OK\n");
        LED = 0; DelayMs(50); LED = 1;
        break;
    }

    case 'C':   /* Current Address Read */
    case 'c':
        data = EEPROM_CurrentAddressRead();
        Uart_SendStr("OK 0x");
        byte2hex(data, hex);
        Uart_SendStr(hex);
        Uart_SendStr("\n");
        break;

    case 'S':   /* Sequential Read */
    case 's':
        i++;
        addr = parse_hex_byte(&cmd_buf[i], &end);
        i = end;
        i = skip_to_comma(i);
        n = 0;
        while (i < cmd_len && cmd_buf[i] >= '0' && cmd_buf[i] <= '9') {
            n = (unsigned char)(n * 10 + (cmd_buf[i] - '0'));
            i++;
        }
        Uart_SendStr("OK [");
        for (i = 0; i < n; i++) {
            EEPROM_SequentialRead((unsigned char)(addr + i), &data, 1);
            if (i > 0) Uart_SendStr(" ");
            Uart_SendStr("0x");
            byte2hex(data, hex);
            Uart_SendStr(hex);
        }
        Uart_SendStr("]\n");
        break;

    case 'T': { /* 测试 5 种方法 */
        unsigned char buf[16];   /* 写和读共用同一缓冲 */
        Uart_SendStr("=== Test All 5 Methods ===\n");

        /* 1. Byte Write */
        Uart_SendStr("[1] Byte Write 0x00=0xAB\n");
        EEPROM_ByteWrite(0x00, 0xAB);

        /* 2. Page Write - 写 16 字节到 0x00 */
        Uart_SendStr("[2] Page Write 0x00=[0x11..0xFF]\n");
        for (i = 0; i < 16; i++) buf[i] = (unsigned char)(0x11 + i * 0x11);
        buf[15] = 0x00;  /* 末字节 */
        EEPROM_PageWrite(0x00, buf, 16);

        /* 3. Current Address Read - 此时内部地址在 0x10 */
        Uart_SendStr("[3] Current Address Read (after write addr=0x10):\n");
        data = EEPROM_CurrentAddressRead();
        Uart_SendStr("    -> 0x");
        byte2hex(data, hex); Uart_SendStr(hex);
        Uart_SendStr(" (expect 0x00, because 0x10 not yet written)\n");

        /* 4. Random Read */
        Uart_SendStr("[4] Random Read 0x05:\n");
        data = EEPROM_RandomRead(0x05);
        Uart_SendStr("    -> 0x");
        byte2hex(data, hex); Uart_SendStr(hex);
        Uart_SendStr(" (expect 0x66)\n");

        /* 5. Sequential Read 0x00..0x0F (16 字节 = 一整页) */
        Uart_SendStr("[5] Sequential Read 0x00-0x0F (16 bytes):\n    [");
        EEPROM_SequentialRead(0x00, buf, 16);
        Uart_SendStr("0x");
        for (i = 0; i < 16; i++) {
            if (i > 0) Uart_SendStr(i % 8 == 0 ? "\n     0x" : " 0x");
            byte2hex(buf[i], hex);
            Uart_SendStr(hex);
        }
        Uart_SendStr("\n]\n");
        Uart_SendStr("=== Done ===\n");
        break;
    }

    default:
        Uart_SendStr("ERR: Unknown cmd\n");
        break;
    }
}

static void poll_cmd(void)
{
    unsigned char ch;
    while (Uart_ReadByte(&ch)) {
        if (ch == '\n') {
            cmd_buf[cmd_len] = '\0';
            process_cmd();
            cmd_len = 0;
        } else if (ch != '\r' && cmd_len < CMD_BUF_SIZE - 1) {
            cmd_buf[cmd_len++] = ch;
        }
    }
}

void main(void)
{
    P34 = 1;
    LED = 1;

    EEPROM_Init();
    /* 注意: 这里仍然显式调用 Uart_Init() 以保持原行为
     * 如果注释掉, Log() 会自动初始化 */
    Uart_Init();

    /* 测试新的 Log API - 自动追加 \r\n */
    Log("=== 08-eeprom Ready ===");
    Log("Build with Log API, auto-init test...");
    Log("Send '?' for help, 'T' to test all");

    while (1) {
        poll_cmd();
    }
}
