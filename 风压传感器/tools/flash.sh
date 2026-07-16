#!/usr/bin/env bash
#
# 风压传感器固件烧录脚本（Linux / Arch Linux）
#
# 依赖:
#   - stcgal (Python): pip install stcgal 或 python -m stcgal
#   - 串口驱动: CH340/FTDI/CP210x (需已连接)
#
# 用法:
#   ./tools/flash.sh                          # 自动检测串口
#   ./tools/flash.sh -p /dev/ttyUSB0          # 指定串口
#   ./tools/flash.sh -f build/SensorBoard.hex # 指定固件文件 (ihx 或 hex)
#   ./tools/flash.sh -p /dev/ttyUSB0 -f build/SensorBoard.hex
#
# 注意:
#   STC 下载需手动断电重上电，在提示 "Power cycle the MCU" 时操作。
#   若自动检测串口失败，用 -p 指定，如:   ./tools/flash.sh -p /dev/ttyUSB0
#

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# ---- 参数解析 ----
PORT=""
FILE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p) PORT="$2";  shift 2 ;;
        -f) FILE="$2";  shift 2 ;;
        *)  echo "用法: $0 [-p 串口] [-f 固件文件]" >&2; exit 1 ;;
    esac
done

# ---- 默认固件文件 ----
if [ -z "$FILE" ]; then
    # 优先用 hex，其次 ihx
    for ext in hex ihx; do
        candidate="$ROOT/build/SensorBoard.$ext"
        if [ -f "$candidate" ]; then
            FILE="$candidate"
            break
        fi
    done
fi

if [ -z "$FILE" ] || [ ! -f "$FILE" ]; then
    echo "错误: 未找到固件文件，请先运行 ./tools/build.sh" >&2
    exit 1
fi
echo "固件: $FILE"

# ---- 检测 stcgal ----
if ! python -m stcgal --help &>/dev/null; then
    echo "错误: 未找到 stcgal。请安装: pip install stcgal" >&2
    exit 1
fi

# ---- 自动检测串口（若未指定） ----
if [ -z "$PORT" ]; then
    CANDIDATES=()
    # 常见 USB 串口设备
    for dev in /dev/ttyUSB* /dev/ttyACM* /dev/ttyS0; do
        [ -e "$dev" ] && CANDIDATES+=("$dev")
    done

    if [ "${#CANDIDATES[@]}" -eq 0 ]; then
        echo "错误: 未检测到串口设备。请用 -p 指定，如: $0 -p /dev/ttyUSB0" >&2
        exit 1
    elif [ "${#CANDIDATES[@]}" -eq 1 ]; then
        PORT="${CANDIDATES[0]}"
        echo "串口: $PORT (自动检测)"
    else
        echo "检测到多个串口设备:"
        for i in "${!CANDIDATES[@]}"; do
            echo "  [$i] ${CANDIDATES[$i]}"
        done
        read -r -p "请选择串口编号: " idx
        PORT="${CANDIDATES[$idx]}"
    fi
else
    echo "串口: $PORT (用户指定)"
fi

# ---- 烧录 ----
echo ""
echo "开始烧录 ..."
echo "提示: 请在提示 \"Power cycle the MCU\" 时，"
echo "      断开目标板电源后重新上电。"
echo ""
python -m stcgal -p "$PORT" -P auto -t 11059 "$FILE"

echo ""
echo "烧录完成。"
