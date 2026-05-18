#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "nlohmann/json.hpp"
#include "DatabaseManager.h"
#include <memory>

using json = nlohmann::json;

// detection.service.ts의 DetectionService 클래스에 대응
class DetectionService {
public:
    DetectionService();
    // saveDetection() 에 대응
    json saveDetection(const json& data);

    // getAllLogs() 에 대응 (나중에 HTTP로 노출할 예정)
    std::vector<json> getAllLogs() const;

    // 기기 목록 반환
    json getDevices() const;

    // 기기 동적 등록
    void addDevice(const json& device);

    // 모의 카메라 상태 업데이트 (Test Mode)
    void updateMockCameraState(int camera_id, bool enabled);

    // 카메라 탐지 타겟 설정 업데이트
    void updateCameraTargets(int camera_id, bool mouse, bool cockroach);

    // 목데이터 DB 시딩
    void seedMockData();

private:
    std::unique_ptr<DatabaseManager> dbManager_;
    std::vector<json> detectionLogs_;   
    mutable std::mutex mutex_;          
    json mockDevices_;                  // 인메모리 기기 상태 저장용
};
