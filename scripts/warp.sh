#!/bin/bash
# 用法：在 ~/.bashrc 加入  source /path/to/warp.sh

# ==========================================
# 1. 資料收集：Hook 系統預設的 cd 指令
# ==========================================
cd() {
    builtin cd "$@"
    local exit_status=$?

    if [ $exit_status -eq 0 ]; then
        command warp --add "$PWD" > /dev/null 2>&1
    fi

    return $exit_status
}

# ==========================================
# 2. 執行跳轉：建立 warp 指令
# ==========================================
# 注意：這裡一律呼叫上面的 cd 函式（不是 builtin cd），
# 這樣每次跳轉成功後 hook 才會記錄分數。
warp() {
    case "$1" in
        --list|--clean)
            command warp "$1"
            return
            ;;
    esac

    # 沒給參數：跳回家目錄
    if [ -z "$1" ]; then
        cd ~
        return
    fi

    # 輸入本身就是真實存在的資料夾：直接跳
    if [ -d "$1" ]; then
        cd -- "$1"
        return
    fi

    # 錯誤訊息由 C++ 走 stderr 直接顯示，這裡只負責判斷成敗
    local target_dir
    target_dir=$(command warp "$1") || return 1
    [ -d "$target_dir" ] || return 1

    cd -- "$target_dir"
}
