#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// detection.service.ts의 DetectionService 클래스에 대응
class DetectionService {
public:
    // saveDetection() 에 대응
    json saveDetection(const json& data);

    // getAllLogs() 에 대응 (나중에 HTTP로 노출할 예정)
    std::vector<json> getAllLogs() const;

private:
    std::vector<json> detectionLogs_;   // private detectionLogs: any[]
    mutable std::mutex mutex_;          // 멀티스레드 안전을 위한 뮤텍스
};
