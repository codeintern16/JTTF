#!/bin/bash

# ==========================================
# 1. 資料收集：Hook (攔截) 系統預設的 cd 指令
# ==========================================
cd() {
    builtin cd "$@"
    local exit_status=$?

    # 【核心修改】：改成 command warp，呼叫系統安裝的執行檔
    if [ $exit_status -eq 0 ]; then
        command warp --add "$PWD" > /dev/null 2>&1
    fi

    return $exit_status
}

# ==========================================
# 2. 執行跳轉：建立 warp 指令
# ==========================================
warp() {
    # 攔截 --list 和 --clean，直接交給 C++ 處理並顯示結果，不需要跳轉
    if [ "$1" == "--list" ] || [ "$1" == "--clean" ]; then
        command warp "$1"
        return
    fi

    # 如果只打 warp 沒給參數，就預設跳回家目錄
    if [ -z "$1" ]; then
        builtin cd ~
        return
    fi

    # 【新增防呆】：如果輸入的本身就是一個真實存在的資料夾，直接跳過去！
    if [ -d "$1" ]; then
        builtin cd "$1"
        return
    fi

    # 【核心修改】：改成 command warp，避免 Bash 無窮迴圈呼叫自己
    local target_dir=$(command warp "$1")

    # 驗證 C++ 回傳的路徑是不是一個真的存在的目錄 (-d)
    if [ -d "$target_dir" ]; then
        builtin cd "$target_dir"
    else
        # 找不到時，把 C++ 印出的錯誤訊息或提示顯示出來
        echo "$target_dir"
    fi
}
