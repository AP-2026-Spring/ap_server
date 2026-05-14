#include "DetectionService.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// 현재 시각을 ISO 8601 문자열로 반환 (new Date().toISOString() 에 대응)
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    // UTC 기준 ISO 8601 포맷
    oss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

// saveDetection(data) 에 대응
json DetectionService::saveDetection(const json& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 새 레코드 생성: { id, ...data, timestamp }
    json newRecord = data;
    newRecord["id"]        = static_cast<int>(detectionLogs_.size()) + 1;
    newRecord["timestamp"] = getCurrentTimestamp();

    detectionLogs_.push_back(newRecord);

    // console.log / console.table 에 대응
    std::cout << "--- 새로운 데이터 저장됨 ---\n";
    std::cout << newRecord.dump(2) << "\n";  // pretty-print (들여쓰기 2칸)
    std::cout << "현재 총 기록 수: " << detectionLogs_.size() << "개\n";

    return newRecord;
}

// getAllLogs() 에 대응
std::vector<json> DetectionService::getAllLogs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return detectionLogs_;
}
