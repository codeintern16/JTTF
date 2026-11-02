#include "side.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono> // 處理 UNIX 時間
#include <cctype>
#include <cstdlib>
using namespace std;
namespace fs = filesystem;

// 1. 模糊匹配演算法 (修正過度匹配問題)
bool isFuzzyMatch(const std::string& query, const std::string& path) {
    if (query.empty()) return true;

    // 【修復核心】：把 "/home/tux/Downloads/fakegroup1"
    // 截斷成只剩 "fakegroup1" 再進行比對
    std::string basename = fs::path(path).filename().string();

    size_t q_idx = 0;
    for (char c : basename) {
        if (std::tolower(c) == std::tolower(query[q_idx])) {
            q_idx++;
            // 如果 query 的字元都找完了，代表匹配成功
            if (q_idx == query.length()) return true;
        }
    }
    return false;
}

// 2. 備用方案：動態決定掃描根目錄或家目錄
std::string searchFallback(std::string query) {
    string search_root;

    // 【實作你的邏輯】：有 / 開頭從根目錄找，否則從家目錄找
    if (!query.empty() && query[0] == '/') {
        search_root = "/";
        query = query.substr(1); // 拔掉斜線，用剩下的字眼去模糊匹配
    } else {
        const char* home_dir = getenv("HOME");
        search_root = home_dir ? home_dir : "/";
    }

    auto options = fs::directory_options::skip_permission_denied;
    string best_match = "";

    try {
        for (auto it = fs::recursive_directory_iterator(search_root, options);
             it != fs::recursive_directory_iterator(); ++it) {

            string current_path = it->path().string();
            string filename = it->path().filename().string();

            // 【OS 陷阱防護】：如果是掃描根目錄，絕對要剪枝 (Pruning) 避開虛擬檔案系統
            if (search_root == "/") {
                if (current_path == "/proc" || current_path == "/sys" ||
                    current_path == "/dev" || current_path == "/run" || current_path == "/mnt") {
                    it.disable_recursion_pending();
                    continue;
                }
            }

            // 略過隱藏資料夾，加快掃描速度
            if (filename[0] == '.') {
                it.disable_recursion_pending();
                continue;
            }

            if (it->is_directory()) {
                if (isFuzzyMatch(query, filename)) {
                    best_match = current_path;
                    break;
                }
            }
        }
    } catch (...) {}

    return best_match;
}

// 取得 UNIX 時間 (秒)
long long getCurrentTime() {
    return chrono::duration_cast<chrono::seconds>(
        chrono::system_clock::now().time_since_epoch()).count();
}

// 3. 載入並反序列化歷史紀錄
vector<Record> loadFrecencyHistory() {
    vector<Record> records;
    const char* home_dir = getenv("HOME");
    if (!home_dir) return records;

    ifstream file(string(home_dir) + "/.warp_history");
    string line;

    // 預期格式: /home/tux/ctf|15.5|1730547960
    while (getline(file, line)) {
        size_t pos1 = line.find('|');
        size_t pos2 = line.rfind('|');
        if (pos1 != string::npos && pos1 != pos2) {
            try {
                Record r;
                r.path = line.substr(0, pos1);
                r.score = stod(line.substr(pos1 + 1, pos2 - pos1 - 1));
                r.timestamp = stoll(line.substr(pos2 + 1));
                records.push_back(r);
            } catch (...) {} // 略過壞掉的紀錄
        }
    }
    return records;
}

// 4. 更新分數、老化衰減、寫回檔案 (Cache Replacement & Pruning)
void addOrUpdateHistory(const string& path) {
    auto records = loadFrecencyHistory();
    bool found = false;
    long long now = getCurrentTime();

    // 命中加 10 分，並更新時間
    for (auto& r : records) {
        if (r.path == path) {
            r.score += 10.0;
            r.timestamp = now;
            found = true;
            break;
        }
    }
    // 新目錄給 10 分初始分數
    if (!found) {
        records.push_back({path, 10.0, now});
    }

    const char* home_dir = getenv("HOME");
    ofstream out(string(home_dir) + "/.warp_history", ios::trunc);

    for (auto& r : records) {
        // 【核心邏輯】：所有人分數衰減 5% (模擬老化)
        r.score *= 0.95;

        // 【核心邏輯】：分數低於 1 的直接淘汰，解決檔案無限膨脹與快取污染
        if (r.score >= 1.0) {
            out << r.path << "|" << r.score << "|" << r.timestamp << "\n";
        }
    }
}
