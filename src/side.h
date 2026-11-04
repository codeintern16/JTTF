#pragma once
#include <string>
#include <vector>

struct Record {
    std::string path;
    double score;
    long long timestamp;
};

int getFuzzyScore(const std::string& query, const std::string& path);
std::string searchFallback(const std::string& query);
std::vector<Record> loadFrecencyHistory();
bool saveFrecencyHistory(const std::vector<Record>& records);
void addOrUpdateHistory(const std::string& path);
