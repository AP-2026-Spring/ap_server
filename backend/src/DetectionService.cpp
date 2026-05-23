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
    }

    // Initialize mock devices state with default Raspberry Pi device seed
    json defaultDev = {
        {"id", 1779125944866},
        {"name", "192.168.0.12"},
        {"ip", "192.168.0.12"},
        {"status", "ready"},
        {"os", "Ubuntu 22.04 LTS"},
        {"network", "1 Gbps"},
        {"cameras", {
            {
                {"id", 1001},
                {"name", "카메라 1 (CSI)"},
                {"enabled", true},
                {"targets", {{"mouse", true}, {"cockroach", true}}}
            },
            {
                {"id", 1002},
                {"name", "카메라 2 (USB)"},
                {"enabled", true},
                {"targets", {{"mouse", true}, {"cockroach", true}}}
            }
        }}
    };
    mockDevices_ = { defaultDev };

    // Seed SQLite database with premium realistic animal detection records if empty!
    auto existingLogs = dbManager_->getAllDetections();
    if (existingLogs.empty()) {
        std::cout << "[Seed] Seeding detections.db with premium realistic animal detection records...\n";
        
        std::string mouse_svg = "data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 640 360\" width=\"100%\" height=\"100%\"><rect width=\"100%\" height=\"100%\" fill=\"%230c0d14\"/><path d=\"M 20 40 L 20 20 L 40 20\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><path d=\"M 620 40 L 620 20 L 600 20\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><path d=\"M 20 320 L 20 340 L 40 340\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><path d=\"M 620 320 L 620 340 L 600 340\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><text x=\"30\" y=\"40\" fill=\"%2322c55e\" font-family=\"monospace\" font-size=\"12\" letter-spacing=\"1\">REC ● 1080P 30FPS</text><text x=\"520\" y=\"40\" fill=\"%2322c55e\" font-family=\"monospace\" font-size=\"12\" letter-spacing=\"1\">CSI-CAM-01</text><rect x=\"220\" y=\"110\" width=\"180\" height=\"120\" fill=\"none\" stroke=\"%23ef4444\" stroke-width=\"2\" stroke-dasharray=\"4\"/><rect x=\"220\" y=\"85\" width=\"130\" height=\"25\" fill=\"%23ef4444\" rx=\"3\"/><text x=\"225\" y=\"102\" fill=\"%23ffffff\" font-family=\"sans-serif\" font-weight=\"bold\" font-size=\"12\">mouse: 89.2%</text><ellipse cx=\"310\" cy=\"170\" rx=\"40\" ry=\"25\" fill=\"%23ffffff\" opacity=\"0.15\"/><circle cx=\"280\" cy=\"150\" r=\"15\" fill=\"%23ffffff\" opacity=\"0.15\"/><circle cx=\"340\" cy=\"150\" r=\"15\" fill=\"%23ffffff\" opacity=\"0.15\"/></svg>";
        std::string cockroach_svg = "data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 640 360\" width=\"100%\" height=\"100%\"><rect width=\"100%\" height=\"100%\" fill=\"%230c0d14\"/><path d=\"M 20 40 L 20 20 L 40 20\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><path d=\"M 620 40 L 620 20 L 600 20\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><path d=\"M 20 320 L 20 340 L 40 340\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><path d=\"M 620 320 L 620 340 L 600 340\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" opacity=\"0.8\"/><text x=\"30\" y=\"40\" fill=\"%2322c55e\" font-family=\"monospace\" font-size=\"12\" letter-spacing=\"1\">REC ● 1080P 30FPS</text><text x=\"520\" y=\"40\" fill=\"%2322c55e\" font-family=\"monospace\" font-size=\"12\" letter-spacing=\"1\">USB-CAM-02</text><rect x=\"180\" y=\"130\" width=\"140\" height=\"110\" fill=\"none\" stroke=\"%2322c55e\" stroke-width=\"2\" stroke-dasharray=\"4\"/><rect x=\"180\" y=\"105\" width=\"160\" height=\"25\" fill=\"%2322c55e\" rx=\"3\"/><text x=\"185\" y=\"122\" fill=\"%23ffffff\" font-family=\"sans-serif\" font-weight=\"bold\" font-size=\"12\">cockroach: 94.7%</text><ellipse cx=\"250\" cy=\"185\" rx=\"30\" ry=\"15\" fill=\"%23ffffff\" opacity=\"0.15\"/></svg>";

        auto now = std::chrono::system_clock::now();
        auto getPastTimeStr = [&](int hoursAgo) {
            auto pastTime = now - std::chrono::hours(hoursAgo);
            auto timeT = std::chrono::system_clock::to_time_t(pastTime);
            std::ostringstream oss;
            oss << std::put_time(std::gmtime(&timeT), "%Y-%m-%dT%H:%M:%SZ");
            return oss.str();
        };

        // Seed beautiful historic data covering 6 different days to populate graphs!
        dbManager_->saveDetection(1001, "mouse", 0.89, 50.0, 60.0, 120.0, 80.0, getPastTimeStr(2), mouse_svg);
        dbManager_->saveDetection(1001, "cockroach", 0.94, 180.0, 120.0, 40.0, 40.0, getPastTimeStr(18), cockroach_svg);
        dbManager_->saveDetection(1002, "mouse", 0.78, 90.0, 70.0, 110.0, 90.0, getPastTimeStr(26), mouse_svg);
        dbManager_->saveDetection(1002, "cockroach", 0.91, 150.0, 80.0, 50.0, 50.0, getPastTimeStr(50), cockroach_svg);
        dbManager_->saveDetection(1001, "mouse", 0.85, 30.0, 40.0, 130.0, 100.0, getPastTimeStr(74), mouse_svg);
        dbManager_->saveDetection(1001, "cockroach", 0.88, 200.0, 140.0, 45.0, 45.0, getPastTimeStr(98), cockroach_svg);
        dbManager_->saveDetection(1001, "mouse", 0.92, 70.0, 50.0, 100.0, 85.0, getPastTimeStr(122), mouse_svg);
        dbManager_->saveDetection(1002, "cockroach", 0.95, 120.0, 100.0, 60.0, 60.0, getPastTimeStr(146), cockroach_svg);

        std::cout << "[Seed] Successfully seeded 8 premium animal detection records into database!\n";
    }
}
 
void DetectionService::seedMockData() {
    // Seed mock data disabled
}

json DetectionService::getDevices() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return mockDevices_;
}

void DetectionService::addDevice(const json& device) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string ip = device.value("ip", "");
    for (const auto& d : mockDevices_) {
        if (d.value("ip", "") == ip) {
            return; // Already exists
        }
    }
    
    json newDev = device;
    if (!newDev.contains("id")) {
        newDev["id"] = static_cast<int>(mockDevices_.size() + 1);
    }
    if (!newDev.contains("status")) {
        newDev["status"] = "ready";
    }
    if (!newDev.contains("cameras") || newDev["cameras"].empty()) {
        // Create default cameras for this new device so they can be managed
        newDev["cameras"] = {
            {
                {"id", static_cast<int>(newDev["id"].get<int>() * 100 + 1)},
                {"name", "Cam 1 (기본)"},
                {"enabled", true},
                {"targets", {{"mouse", true}, {"cockroach", false}}}
            }
        };
    }
    
    std::cout << "[Server] Dynamically registered new device: " << newDev.value("name", "Unknown") << " (" << ip << ")\n";
    mockDevices_.push_back(newDev);
}

bool DetectionService::removeDevice(uint64_t device_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = mockDevices_.begin(); it != mockDevices_.end(); ++it) {
        if (it->contains("id")) {
            uint64_t current_id = 0;
            if (it->at("id").is_number()) {
                current_id = it->at("id").get<uint64_t>();
            } else if (it->at("id").is_string()) {
                try {
                    current_id = std::stoull(it->at("id").get<std::string>());
                } catch (...) {}
            }
            if (current_id == device_id) {
                std::cout << "[Server] Deleting device ID: " << device_id << "\n";
                mockDevices_.erase(it);
                return true;
            }
        }
    }
    return false;
}

void DetectionService::updateMockCameraState(int camera_id, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& device : mockDevices_) {
        if (device.contains("cameras")) {
            for (auto& cam : device["cameras"]) {
                if (cam["id"] == camera_id) {
                    cam["enabled"] = enabled;
                    std::cout << "[Test Mode] Camera " << camera_id << " power state changed to " << (enabled ? "ON" : "OFF") << " in memory.\n";
                    return;
                }
            }
        }
    }
}

void DetectionService::updateCameraTargets(int camera_id, bool mouse, bool cockroach) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& device : mockDevices_) {
        if (device.contains("cameras")) {
            for (auto& cam : device["cameras"]) {
                if (cam["id"] == camera_id) {
                    cam["targets"]["mouse"] = mouse;
                    cam["targets"]["cockroach"] = cockroach;
                    std::cout << "[Server] Camera " << camera_id << " target filters updated: Mouse=" << (mouse ? "ON" : "OFF") << ", Cockroach=" << (cockroach ? "ON" : "OFF") << "\n";
                    return;
                }
            }
        }
    }
}

// saveDetection(data) 에 대응
json DetectionService::saveDetection(const json& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    if(data.contains("confidence") && data["confidence"] < 0.7)
    {
        std::cout << "신호 무시됨 (confidence < 0.7): " << data.dump() << "\n";
        return json{};
    }

    int camera_id = data.contains("camera_id") ? data["camera_id"].get<int>() : 0;
    std::string class_name = data.contains("class_name") ? data["class_name"].get<std::string>() : "unknown";

    // ── 카메라 탐지 타겟 필터링 검사 ──────────────────────────────────
    if (camera_id > 0) {
        for (const auto& device : mockDevices_) {
            if (device.contains("cameras")) {
                for (const auto& cam : device["cameras"]) {
                    if (cam.contains("id") && cam["id"] == camera_id) {
                        if (cam.contains("targets")) {
                            auto targets = cam["targets"];
                            if (class_name == "mouse" && targets.contains("mouse") && !targets["mouse"].get<bool>()) {
                                std::cout << "[Filter] Mouse detection ignored because Mouse filter is OFF for Camera " << camera_id << "\n";
                                return json{};
                            }
                            if (class_name == "cockroach" && targets.contains("cockroach") && !targets["cockroach"].get<bool>()) {
                                std::cout << "[Filter] Cockroach detection ignored because Cockroach filter is OFF for Camera " << camera_id << "\n";
                                return json{};
                            }
                        }
                    }
                }
            }
        }
    }

    std::string timestamp = getCurrentTimestamp();

    // Auto-register device dynamically if camera_id doesn't match any registered device
    if (camera_id > 0) {
        bool found = false;
        for (const auto& device : mockDevices_) {
            if (device.contains("cameras")) {
                for (const auto& cam : device["cameras"]) {
                    if (cam.contains("id") && cam["id"] == camera_id) {
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
        }
        
        if (!found) {
            int device_id = camera_id / 100;
            std::string device_name = "Edge Device " + std::to_string(device_id);
            std::string ip_addr = "192.168.1." + std::to_string(device_id * 10);
            
            json newDev = {
                {"id", device_id},
                {"name", device_name},
                {"ip", ip_addr},
                {"status", "ready"},
                {"os", "Raspberry Pi OS"},
                {"network", "Wi-Fi (Auto)"},
                {"cameras", {
                    {
                        {"id", camera_id},
                        {"name", "Camera " + std::to_string(camera_id)},
                        {"enabled", true},
                        {"targets", {{"mouse", true}, {"cockroach", true}}}
                    }
                }}
            };
            std::cout << "[WS Auto-Register] Registering new edge device: " << device_name << " (" << ip_addr << ") with Cam ID " << camera_id << "\n";
            mockDevices_.push_back(newDev);
        }
    }

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
