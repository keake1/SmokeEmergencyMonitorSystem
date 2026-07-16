#!/usr/bin/env bash
# 烧录 Hostboard.elf 到 STM32F407VET6，烧录后自动复位运行
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ELF="${SCRIPT_DIR}/build/Debug/Hostboard.elf"

if [ ! -f "$ELF" ]; then
    echo "Error: 找不到 $ELF"
    echo "请先执行: cd Hostboard && cmake --build build/Debug"
    exit 1
fi

echo "==> 烧录 $(basename "$ELF") ..."
openocd -f "${SCRIPT_DIR}/flash.cfg" \
        -c "program ${ELF} verify reset exit"
echo "==> 完成"
