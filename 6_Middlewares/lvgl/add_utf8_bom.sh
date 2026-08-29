#!/bin/bash
# ============================================================
# 给 GUI Guider 生成的中文源码补 UTF-8 BOM 脚本
#
# 背景（重要，别再用"转 GB18030"那种做法了）：
#   GUI Guider 生成的 setup_scr_SettingsPage4*.c 和 custom/lv_conf_ext.h
#   内容是 UTF-8，并且原本自带 UTF-8 BOM (EF BB BF)。
#   Keil ARMCC V5 (AC5) 靠文件头 BOM 自动识别 UTF-8，才能正确读出
#   中文字符串字面量、编译通过，且固件里保留 UTF-8 字节 → 屏幕正常显示中文
#   （LVGL 的 LV_TXT_ENC=UTF8 按 UTF-8 渲染）。
#
#   错误做法：iconv 转 GB18030 → 能编译过但固件是 GBK 字节 → 运行时乱码。
#   无 BOM 的 UTF-8 → ARMCC 按系统码页 GBK 误读 → error: #8: missing closing quote。
#
# 用法: ./add_utf8_bom.sh
# GUI Guider 重新生成代码后（生成文件通常带 BOM，一般无需再跑），
# 若文件丢了 BOM，重新运行一次即可。
# ============================================================
set -e

UI_DIR="$(cd "$(dirname "$0")/SleepAid_UI" && pwd)"
cd "$UI_DIR"

# 只处理含中文字符串字面量的源码：屏幕代码 + 自定义配置
# 不要碰 guider_fonts/、images/ 的二进制字形/像素数据
FILES="custom/lv_conf_ext.h generated/setup_scr_SettingsPage4*.c"

for f in $FILES; do
    [ -f "$f" ] || continue

    # 已有 BOM 则跳过
    if [ "$(head -c 3 "$f" | od -An -tx1 | tr -d ' \n')" = "efbbbf" ]; then
        echo "skip (already has BOM): $f"
        continue
    fi

    # 内容必须是合法 UTF-8 才补 BOM；不是 UTF-8（如已变成 GBK）则不处理并提示
    if iconv -f UTF-8 -t UTF-8 "$f" > /dev/null 2>&1; then
        printf '\xef\xbb\xbf' > "$f.bom.tmp"
        cat "$f" >> "$f.bom.tmp"
        mv "$f.bom.tmp" "$f"
        echo "+BOM added: $f"
    else
        echo "SKIP (not UTF-8, check manually): $f"
    fi
done

echo "done"
