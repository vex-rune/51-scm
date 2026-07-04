#!/usr/bin/env python3
"""
8x8 LED 点阵模拟器 (终端版)
模拟 src/05-dot-matrix/ 的显示效果, 直接读 FONT_DATA 数组

用法:
    python tools/sim_matrix.py
    python tools/sim_matrix.py 100     # 100ms 每步 (默认 100)
"""

import sys
import time
import os

# 找到 FONT_DATA 数组, 从 main.c 提取
MAIN_C = os.path.join(os.path.dirname(__file__), '..', 'src', '05-dot-matrix', 'main.c')

def extract_font():
    """从 main.c 提取 FONT_DATA 数组"""
    with open(MAIN_C, 'r', encoding='utf-8') as f:
        text = f.read()

    # 找 FONT_DATA[] = { ... } 区域
    start = text.find('FONT_DATA[] = {')
    if start < 0:
        raise RuntimeError("未找到 FONT_DATA")

    # 找到匹配的 }
    i = text.find('{', start)
    depth = 0
    end = i
    for j in range(i, len(text)):
        if text[j] == '{':
            depth += 1
        elif text[j] == '}':
            depth -= 1
            if depth == 0:
                end = j
                break

    body = text[i+1:end]
    # 提取 0xNN 字节
    bytes_list = []
    for tok in body.split(','):
        tok = tok.strip()
        if tok.startswith('0x') or tok.startswith('0X'):
            bytes_list.append(int(tok, 16))
    return bytes_list

def render(buf8):
    """把 8 字节缓冲渲染成 8x8 字符画"""
    lines = []
    for row in range(8):
        b = buf8[row]
        line = ''
        for col in range(8):
            if b & (0x80 >> col):
                line += '##'
            else:
                line += '  '
        lines.append(line)
    return lines

def main():
    speed_ms = 100
    if len(sys.argv) > 1:
        speed_ms = int(sys.argv[1])

    # 1. 读字模
    font = extract_font()
    char_count = len(font) // 8
    print(f"已加载 {char_count} 个字模 ({len(font)} 字节)")
    print(f"速度: {speed_ms} ms/步\n")

    # 2. 清屏
    os.system('cls' if os.name == 'nt' else 'clear')

    # 3. 滚动循环
    total_steps = char_count * 8 + 8
    for step in range(total_steps):
        buf8 = [0] * 8

        # 拼当前 step 对应的 8 列
        for i in range(8):
            abs_col = step + i
            char_pos = abs_col >> 3
            char_col = abs_col & 0x07

            if char_pos >= char_count:
                continue

            # font[char_pos*8 + row] 8 字节 = 该字符的 8 行
            base = char_pos * 8
            for row in range(8):
                if font[base + row] & (0x80 >> char_col):
                    buf8[row] |= (0x80 >> i)

        # 渲染
        os.system('cls' if os.name == 'nt' else 'clear')
        print(f"=== 8x8 LED 点阵模拟器 ===")
        print(f"step = {step:3d} / {total_steps-1}")
        print('+' + '-' * 16 + '+')
        for line in render(buf8):
            print('|' + line + '|')
        print('+' + '-' * 16 + '+')
        print(f"按 Ctrl+C 停止")

        time.sleep(speed_ms / 1000.0)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n已停止")
