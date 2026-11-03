#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <fstream>   // 【新增】為了開啟與覆寫歷史檔案
#include "side.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    // 攔截 1: Bash 傳來的紀錄請求
    if (string(argv[1]) == "--add" && argc == 3) {
        addOrUpdateHistory(argv[2]);
        return 0;
    }

    // 攔截 2: 印出快取清單與權重
    if (string(argv[1]) == "--list") {
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

    // 【新增】攔截 3: 手動清除不存在的失效路徑
    if (string(argv[1]) == "--clean") {
        auto records = loadFrecencyHistory();
        const char* home_dir = getenv("HOME");
        if (!home_dir) return 1;

        // ios::trunc 代表清空原本的檔案，重新寫入
        ofstream out(string(home_dir) + "/.warp_history", ios::trunc);
        int removed_count = 0;

        for (const auto& r : records) {
            // 只有當資料夾確實存在時，才寫回檔案保留
            if (fs::exists(r.path) && fs::is_directory(r.path)) {
                out << r.path << "|" << r.score << "|" << r.timestamp << "\n";
            } else {
                removed_count++; // 紀錄清掉了幾個無效路徑
            }
        }

        cout << "[Warp] 🧹 Cleanup complete! Removed " << removed_count << " invalid paths.\n";
        return 0;
    }

    string query = argv[1];

    // 強制全域掃描後門
    bool force_scan = false;
    if (!query.empty() && query[0] == '+') {
        force_scan = true;
        query = query.substr(1);
    }

    auto records = loadFrecencyHistory();
    string best_match = "";
    double max_score = -1.0;
    bool found_exact = false;

    // 1. 搜尋快取
    if (!force_scan) {
        for (const auto& r : records) {
            if (fs::exists(r.path) && fs::is_directory(r.path)) {
                if (isFuzzyMatch(query, r.path)) {

                    string basename = fs::path(r.path).filename().string();
                    bool is_exact = (basename == query);

                    if (is_exact && !found_exact) {
                        found_exact = true;
                        max_score = -1.0;
                    }

                    if (found_exact && !is_exact) {
                        continue;
                    }

                    if (r.score > max_score) {
                        max_score = r.score;
                        best_match = r.path;
                    }
                }
            }
        }
    }

    // 2. 全域掃描
    if (best_match.empty()) {
        if (force_scan) {
            cerr << "[Warp] Force scanning the whole area...\n";
        } else {
            cerr << "[Warp] cache miss, scan the whole area...\n";
        }

        best_match = searchFallback(query);

        if (!best_match.empty()) {
            addOrUpdateHistory(best_match);
            cerr << "[Warp] add to cache！\n";
        }
    }

    // 3. 輸出給 Bash 執行
    if (best_match.empty()) {
        cout << "didnt find the match directory: " << query << "\n";
    } else {
        cout << best_match << "\n";
    }

    return 0;
}
