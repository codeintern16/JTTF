#include "side.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
namespace fs = std::filesystem;

// 取得歷史檔路徑；HOME 不存在時回傳空字串
static string historyPath() {
    const char* home = getenv("HOME");
    if (!home || !*home) return "";
    return string(home) + "/.warp_history";
}

// 1. 模糊匹配演算法
extern "C" {
    int get_fuzzy_score_rs(const char* query, const char* path);
}

int getFuzzyScore(const std::string& query, const std::string& path) {
    return get_fuzzy_score_rs(query.c_str(), path.c_str());
}

// 2. 呼叫外部 fd 進行全域掃描（fork + exec，不經過 shell，無 injection 風險）
string searchFallback(const string& query) {
    const char* home = getenv("HOME");
    if (!home || !*home) return "";

    int fds[2];
    if (pipe(fds) != 0) return "";

    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return "";
    }

    if (pid == 0) {  // 子行程
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);

        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        // "--" 避免 query 以 '-' 開頭時被當成選項
        execlp("fd", "fd", "-t", "d", "-H", "-E", ".git",
               "-c", "never", "-1", "--", query.c_str(), home,
               static_cast<char*>(nullptr));
        _exit(127);  // exec 失敗（通常是找不到 fd）
    }

    // 父行程：讀取子行程輸出
    close(fds[1]);
    string result;
    array<char, 256> buffer;
    while (true) {
        ssize_t n = read(fds[0], buffer.data(), buffer.size());
        if (n > 0) {
            result.append(buffer.data(), static_cast<size_t>(n));
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    close(fds[0]);

    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}

    if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
        cerr << "[Warp Error] 找不到 'fd' 指令！請先執行: sudo pacman -S fd\n";
        return "";
    }

    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

// 3. 取得 UNIX 時間 (秒)
long long getCurrentTime() {
    return chrono::duration_cast<chrono::seconds>(
        chrono::system_clock::now().time_since_epoch()).count();
}

// 4. 載入並反序列化歷史紀錄
vector<Record> loadFrecencyHistory() {
    vector<Record> records;
    string hist = historyPath();
    if (hist.empty()) return records;

    ifstream file(hist);
    string line;

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
            } catch (...) {}
        }
    }
    return records;
}

// 5. 原子寫入歷史檔（先寫暫存檔，再 rename 覆蓋）
bool saveFrecencyHistory(const vector<Record>& records) {
    string hist = historyPath();
    if (hist.empty()) return false;

    // 暫存檔名帶 pid，避免多個 shell 同時寫入時互相踩到
    string tmp = hist + ".tmp." + to_string(getpid());
    {
        ofstream out(tmp, ios::trunc);
        if (!out) return false;
        for (const auto& r : records) {
            out << r.path << "|" << r.score << "|" << r.timestamp << "\n";
        }
        out.flush();
        if (!out) {
            error_code ec;
            fs::remove(tmp, ec);
            return false;
        }
    }

    error_code ec;
    fs::rename(tmp, hist, ec);
    if (ec) {
        fs::remove(tmp, ec);
        return false;
    }
    return true;
}

// 6. 更新分數、老化衰減、寫回檔案
void addOrUpdateHistory(const string& path) {
    auto records = loadFrecencyHistory();
    bool found = false;
    long long now = getCurrentTime();

    // 全體先老化衰減 5%
    for (auto& r : records) r.score *= 0.95;

    // 命中的目標加上完整的 10 分
    for (auto& r : records) {
        if (r.path == path) {
            r.score += 10.0;
            r.timestamp = now;
            found = true;
            break;
        }
    }
    if (!found) records.push_back({path, 10.0, now});

    // 分數過低的紀錄直接淘汰
    records.erase(remove_if(records.begin(), records.end(),
                            [](const Record& r) { return r.score < 1.0; }),
                  records.end());

    saveFrecencyHistory(records);
}
