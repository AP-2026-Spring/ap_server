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

// Constructor
DetectionService::DetectionService() {
    dbManager_ = std::make_unique<DatabaseManager>("detections.db");
    if (!dbManager_->initialize()) {
        std::cerr << "Failed to initialize database!\n";
    } else {
        seedMockData();
    }
}

void DetectionService::seedMockData() {
    auto logs = dbManager_->getAllDetections();
    if (logs.empty()) {
        std::cout << "[Server] Database is empty. Seeding mock data...\n";
        dbManager_->saveDetection(101, "mouse", 0.92, 120.5, 85.0, 40.0, 25.0, "2026-05-18 14:15:22", "snapshot.png");
        dbManager_->saveDetection(101, "mouse", 0.88, 150.0, 90.0, 38.0, 22.0, "2026-05-18 10:05:11", "");
        dbManager_->saveDetection(101, "cockroach", 0.75, 200.0, 200.0, 10.0, 10.0, "2026-05-17 22:30:00", "");
    }
}

json DetectionService::getDevices() const {
    return {
        {
            {"id", 1},
            {"name", "메인 서버(주방)"},
            {"ip", "192.168.1.10"},
            {"status", "ready"},
            {"os", "Ubuntu 22.04 LTS"},
            {"network", "1 Gbps"},
            {"cameras", {
                {
                    {"id", 101},
                    {"name", "Cam 1 부엌"},
                    {"enabled", true},
                    {"targets", {{"mouse", true}, {"cockroach", false}}}
                },
                {
                    {"id", 102},
                    {"name", "Cam 2 식재료 창고"},
                    {"enabled", false},
                    {"targets", {{"mouse", true}, {"cockroach", true}}}
                }
            }}
        },
        {
            {"id", 2},
            {"name", "보조 서버(식당)"},
            {"ip", "192.168.1.20"},
            {"status", "offline"},
            {"os", "Ubuntu 20.04 LTS"},
            {"network", "100 Mbps"},
            {"cameras", json::array()}
        }
    };
}

// saveDetection(data) 에 대응
json DetectionService::saveDetection(const json& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    if(data.contains("confidence") && data["confidence"] < 0.7)
    {
        std::cout << "신호 무시됨 (confidence < 0.7): " << data.dump() << "\n";
        return json{};
    }

    std::string timestamp = getCurrentTimestamp();

    int camera_id = data.contains("camera_id") ? data["camera_id"].get<int>() : 0;
    std::string class_name = data.contains("class_name") ? data["class_name"].get<std::string>() : "unknown";
    double confidence = data.contains("confidence") ? data["confidence"].get<double>() : 0.0;
    
    double x = data.contains("x") ? data["x"].get<double>() : 0.0;
    double y = data.contains("y") ? data["y"].get<double>() : 0.0;
    double w = data.contains("w") ? data["w"].get<double>() : 0.0;
    double h = data.contains("h") ? data["h"].get<double>() : 0.0;
    
    std::string image_data = data.contains("image_data") ? data["image_data"].get<std::string>() : "";

    int id = dbManager_->saveDetection(camera_id, class_name, confidence, x, y, w, h, timestamp, image_data);

    // 새 레코드 생성: { id, ...data, timestamp }
    json newRecord = data;
    newRecord["id"]        = id;
    newRecord["timestamp"] = timestamp;

    detectionLogs_.push_back(newRecord);

    // console.log / console.table 에 대응
    std::cout << "--- 새로운 데이터 저장됨 (DB ID: " << id << ") ---\n";
    std::cout << newRecord.dump(2) << "\n";  // pretty-print (들여쓰기 2칸)
    std::cout << "현재 메모리 기록 수: " << detectionLogs_.size() << "개\n";

    return newRecord;
}

// getAllLogs() 에 대응
std::vector<json> DetectionService::getAllLogs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    // DB에서 전체 조회하여 반환
    return dbManager_->getAllDetections();
}
