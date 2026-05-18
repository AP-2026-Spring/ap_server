#include <iostream>
#include <cassert>
#include <cstdio>
#include "DatabaseManager.h"

int main() {
    std::cout << "Starting DatabaseManager tests...\n";

    // 1. 테스트용 DB 파일 경로
    std::string test_db_path = "test_detections.db";
    
    // 이전에 남아있던 테스트 DB 파일 삭제 (초기화)
    std::remove(test_db_path.c_str());

    // 2. DatabaseManager 인스턴스 생성 및 초기화
    DatabaseManager dbManager(test_db_path);
    bool initResult = dbManager.initialize();
    assert(initResult == true && "Database initialization failed.");
    std::cout << "[OK] Database initialization\n";

    // 3. 데이터 삽입 테스트
    std::string test_class = "person";
    double test_conf = 0.95;
    double tx = 10.5, ty = 20.0, tw = 100.0, th = 200.5;
    std::string test_time = "2026-05-18T12:00:00Z";
    std::string test_img = "base64_encoded_dummy_data";

    int id1 = dbManager.saveDetection(101, test_class, test_conf, tx, ty, tw, th, test_time, test_img);
    assert(id1 > 0 && "First insertion failed.");
    std::cout << "[OK] First record insertion (ID: " << id1 << ")\n";

    // 신뢰도가 낮은 데이터 (예: 자동차) 삽입
    int id2 = dbManager.saveDetection(102, "car", 0.45, 0, 0, 50, 50, "2026-05-18T12:05:00Z", "");
    assert(id2 > 0 && "Second insertion failed.");
    std::cout << "[OK] Second record insertion (ID: " << id2 << ")\n";

    // 4. 데이터 조회 테스트
    auto records = dbManager.getAllDetections();
    assert(records.size() == 2 && "Should retrieve exactly 2 records.");
    std::cout << "[OK] Retrieve all records (Count: " << records.size() << ")\n";

    // 첫 번째 레코드 값 검증
    json first_record = records[0];
    assert(first_record["id"] == id1);
    assert(first_record["class_name"] == test_class);
    assert(first_record["confidence"] == test_conf);
    assert(first_record["x"] == tx);
    assert(first_record["w"] == tw);
    assert(first_record["timestamp"] == test_time);
    assert(first_record["image_data"] == test_img);
    std::cout << "[OK] First record data validation\n";

    // 두 번째 레코드 값 검증
    json second_record = records[1];
    assert(second_record["id"] == id2);
    assert(second_record["class_name"] == "car");
    assert(second_record["confidence"] == 0.45);
    assert(second_record["image_data"] == "");
    std::cout << "[OK] Second record data validation\n";

    // 5. 테스트 종료 후 임시 파일 정리
    std::remove(test_db_path.c_str());
    std::cout << "[OK] Clean up test database file\n";

    std::cout << "\nAll DatabaseManager tests passed successfully!\n";
    return 0;
}
