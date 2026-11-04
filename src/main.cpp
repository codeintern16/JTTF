#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#include "side.h"

using namespace std;
namespace fs = std::filesystem;

// 不丟例外的目錄檢查（權限不足等情況回傳 false）
static bool isDir(const string& p) {
    error_code ec;
    return fs::is_directory(p, ec);
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    const string arg = argv[1];

    // 攔截 1: Bash cd hook 傳來的紀錄請求
    if (arg == "--add") {
        if (argc != 3) return 1;
        addOrUpdateHistory(argv[2]);
        return 0;
    }

    // 攔截 2: 印出快取清單與權重
    if (arg == "--list") {
        auto records = loadFrecencyHistory();
        sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
            return a.score > b.score;
        });

        cout << "\n================ Warp Cache List ================\n";
        cout << left << setw(10) << "Score" << "Path\n";
        cout << "-------------------------------------------------\n";
        for (const auto& r : records) {
            cout << left << fixed << setprecision(1) << setw(10)
                 << r.score << r.path << "\n";
        }
        cout << "=================================================\n\n";
        return 0;
    }

    // 攔截 3: 清除不存在的失效路徑
    if (arg == "--clean") {
        auto records = loadFrecencyHistory();
        vector<Record> kept;
        for (const auto& r : records) {
            if (isDir(r.path)) kept.push_back(r);
        }
        size_t removed = records.size() - kept.size();

        if (removed > 0 && !saveFrecencyHistory(kept)) {
            cerr << "[Warp Error] 無法寫入歷史檔\n";
            return 1;
        }
        cout << "[Warp] 🧹 Cleanup complete! Removed " << removed << " invalid paths.\n";
        return 0;
    }

    // 未知選項
    if (arg.rfind("--", 0) == 0) {
        cerr << "[Warp] unknown option: " << arg << "\n";
        return 1;
    }

    string query = arg;

    // 強制全域掃描後門
    bool force_scan = false;
    if (!query.empty() && query[0] == '+') {
        force_scan = true;
        query.erase(0, 1);
    }
    if (query.empty()) {
        cerr << "[Warp] empty query\n";
        return 1;
    }

    // ==========================================
    // 【全新升級】：雙層決策變數
    // ==========================================
    string best_match;
    int best_tier = -1;           // 第一層：字串匹配等級 (0~3，3為精確命中)
    double best_frecency = -1.0;  // 第二層：歷史熱度 (Frecency 分數)

    // 1. 搜尋快取
    if (!force_scan) {
        auto records = loadFrecencyHistory();
        for (const auto& r : records) {
            if (!isDir(r.path)) continue;

            // 呼叫 Rust 引擎，獲取 0 ~ 3 的匹配等級
            int tier = getFuzzyScore(query, r.path);
            if (tier == 0) continue; // 毫無關聯，直接淘汰

            // 【核心決策】：先比 Tier，如果 Tier 一樣，再比 Frecency 分數
            if (tier > best_tier || (tier == best_tier && r.score > best_frecency)) {
                best_tier = tier;
                best_frecency = r.score;
                best_match = r.path;
            }
        }
    }

    // 2. 全域掃描（分數由 warp.sh 的 cd hook 在跳轉成功後統一記錄）
    if (best_match.empty()) {
        cerr << (force_scan ? "[Warp] Force scanning the whole area...\n"
                            : "[Warp] cache miss, scan the whole area...\n");
        best_match = searchFallback(query);
    }

    // 3. stdout 只輸出路徑；找不到走 stderr + 非 0 exit code
    if (best_match.empty()) {
        cerr << "[Warp] no match found: " << query << "\n";
        return 1;
    }
    cout << best_match << "\n";
    return 0;
}
