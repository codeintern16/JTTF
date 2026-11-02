#pragma once
#include <string>
#include <vector>

struct Record {
    std::string path;
    double score;
    long long timestamp;
};

bool isFuzzyMatch(const std::string& query, const std::string& path);
std::string searchFallback(std::string query);
std::vector<Record> loadFrecencyHistory();
void addOrUpdateHistory(const std::string& path);
