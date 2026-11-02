#include <iostream>
#include <string>
#include <filesystem>
#include "side.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    if (string(argv[1]) == "--add" && argc == 3) {
        addOrUpdateHistory(argv[2]);
        return 0;
    }

    string query = argv[1];
    auto records = loadFrecencyHistory();

    string best_match = "";
    double max_score = -1.0;

    // 【核心新增】：絕對命中標記
    bool found_exact = false;

    // 1. 搜尋快取
    for (const auto& r : records) {
        if (fs::exists(r.path) && fs::is_directory(r.path)) {
            if (isFuzzyMatch(query, r.path)) {

                string basename = fs::path(r.path).filename().string();
                bool is_exact = (basename == query);

                // 狀態機切換：如果這是我們找到的第一個「完全命中」
                if (is_exact && !found_exact) {
                    found_exact = true;
                    max_score = -1.0; // 分數歸零！把之前那些靠模糊湊出來的垃圾紀錄全部作廢
                }

                // 嚴格篩選：如果已經進入「完全命中模式」，但當下這個只是「模糊命中」，直接略過
                if (found_exact && !is_exact) {
                    continue;
                }

                // 正常的比分數 (只在同量級之間比：模糊 vs 模糊，或是 完全命中 vs 完全命中)
                if (r.score > max_score) {
                    max_score = r.score;
                    best_match = r.path;
                }
            }
        }
    }

    // 2. 備用全域搜
    if (best_match.empty()) {
        cerr << "[Warp] cache miss，scan the whole area...\n";
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
