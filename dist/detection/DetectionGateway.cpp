#include "DetectionGateway.h"
#include <iostream>

DetectionGateway::DetectionGateway(DetectionService& service)
    : detectionService_(service) {}

void DetectionGateway::registerRoutes(uWS::App& app) {

    // @WebSocketGateway(8081) + @SubscribeMessage('detection_signal') 에 대응
    // Socket.io의 네임스페이스 '/'에 '/detection' 경로로 매핑
    app.ws</*PerSocketData=*/int>("/detection", {

        // --- 연결 이벤트 ---
        .open = [](auto* ws) {
            std::cout << "[Gateway] 클라이언트 연결됨\n";
        },

        // --- 메시지 수신: handleSignal() 에 대응 ---
        // Socket.io의 emit('detection_signal', data) 를
        // WebSocket 텍스트 메시지로 수신
        .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode) {
            if (opCode != uWS::OpCode::TEXT) return;

            try {
                // 수신한 JSON 파싱
                json data = json::parse(message);

                std::cout << "Received signal from client: " << data.dump() << "\n";

                // 서비스에 저장 (this->detectionService_.saveDetection(data))
                json saved = detectionService_.saveDetection(data);

                // 클라이언트에게 { received: true } 응답 전송
                json response = {
                    {"received", true},
                    {"id",       saved["id"]},
                    {"timestamp",saved["timestamp"]}
                };
                ws->send(response.dump(), uWS::OpCode::TEXT);

            } catch (const json::parse_error& e) {
                std::cerr << "[Gateway] JSON 파싱 실패: " << e.what() << "\n";

                // 파싱 실패 시 에러 응답
                json errResp = {{"received", false}, {"error", "Invalid JSON"}};
                ws->send(errResp.dump(), uWS::OpCode::TEXT);
            }
        },

        // --- 연결 해제 이벤트 ---
        .close = [](auto* ws, int code, std::string_view reason) {
            std::cout << "[Gateway] 클라이언트 연결 해제 (code=" << code << ")\n";
        }
    });

    // HTTP GET /logs → getAllLogs() 결과 반환
    app.get("/logs", [this](auto* res, auto* req) {
        auto logs = detectionService_.getAllLogs();
    
        json response = json::array();
        
        for (const auto& log : logs) {
            response.push_back(log);
        }
    
        res->writeHeader("Content-Type", "application/json")
           ->writeHeader("Access-Control-Allow-Origin", "*")
           ->end(response.dump(2));
});
}
