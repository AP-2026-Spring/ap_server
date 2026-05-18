#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "nlohmann/json.hpp"
#include <sqlite3.h>

using json = nlohmann::json;

class DatabaseManager {
public:
    DatabaseManager(const std::string& db_path);
    ~DatabaseManager();

    bool initialize();

    // detection 데이터 저장 (id 반환, 실패시 -1)
    int saveDetection(int camera_id, const std::string& class_name, double confidence,
                      double x, double y, double w, double h,
                      const std::string& timestamp, const std::string& image_data);

    // 저장된 모든 기록 가져오기
    std::vector<json> getAllDetections();

private:
    std::string db_path_;
    sqlite3* db_;
    std::mutex db_mutex_;
};
