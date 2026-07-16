#!/usr/bin/env bash
#
# 风压传感器固件编译脚本（Linux / Arch Linux）
#
# 依赖: sdcc (>= 4.0, 支持 mcs51)
#   sudo pacman -S sdcc
#
# 用法: ./tools/build.sh
# 输出: build/SensorBoard.ihx
#       build/SensorBoard.hex
#

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"
SRC="$ROOT/src"
INC="$ROOT/include"

# ---- 查找 SDCC ----
SDCC="${SDCC:-sdcc}"

if ! command -v "$SDCC" &>/dev/null; then
    echo "错误: 未找到 sdcc。请安装: sudo pacman -S sdcc" >&2
    exit 1
fi

# 验证 MCS-51 支持
if ! "$SDCC" --version 2>&1 | grep -qw mcs51; then
    echo "错误: sdcc 不支持 mcs51 架构，请安装官方 SDCC (sdcc.sourceforge.net)" >&2
    exit 1
fi

echo "SDCC: $("$SDCC" --version 2>&1 | head -1)"

mkdir -p "$BUILD"

SOURCES=(
    main.c
    board.c
    crc16.c
    display.c
    pressure.c
    sensor_modbus.c
    uart.c
)

OBJECTS=()
for s in "${SOURCES[@]}"; do
    name="${s%.c}"
    echo "  编译 $s ..."
    "$SDCC" -mmcs51 --std-c99 --model-small --out-fmt-ihx \
        -I"$INC" \
        -c "$SRC/$s" \
        -o "$BUILD/$name.rel"
    OBJECTS+=("$BUILD/$name.rel")
done

echo "  链接 ..."
"$SDCC" -mmcs51 --std-c99 --model-small --out-fmt-ihx \
    -o "$BUILD/SensorBoard.ihx" \
    "${OBJECTS[@]}"

# 生成 HEX 文件（供 stcgal 烧录）
PACKIHX="$(dirname "$(command -v "$SDCC")")/packihx"
if [ -x "$PACKIHX" ]; then
    "$PACKIHX" "$BUILD/SensorBoard.ihx" > "$BUILD/SensorBoard.hex"
    echo "生成: $BUILD/SensorBoard.hex"
fi

echo "完成: $BUILD/SensorBoard.ihx"
