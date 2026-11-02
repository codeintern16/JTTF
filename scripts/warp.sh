#!/bin/bash

# ==========================================
# 1. 資料收集：Hook (攔截) 系統預設的 cd 指令
# ==========================================
cd() {
    builtin cd "$@"
    local exit_status=$?

    # 成功切換後，呼叫 C++ 程式來更新分數與時間 (把輸出丟進黑洞避免干擾畫面)
    if [ $exit_status -eq 0 ]; then
        ~/warp/build/warp --add "$PWD" > /dev/null 2>&1
    fi

    return $exit_status
}

# ==========================================
# 2. 執行跳轉：建立 warp 指令
# ==========================================
warp() {
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

    # 呼叫你編譯好的 C++ 程式幫我們做高速搜尋
    local target_dir=$(~/warp/build/warp "$1")

    # 驗證 C++ 回傳的路徑是不是一個真的存在的目錄 (-d)
    if [ -d "$target_dir" ]; then
        builtin cd "$target_dir"
    else
        # 找不到時，把 C++ 印出的錯誤訊息或提示顯示出來
        echo "$target_dir"
    fi
}
