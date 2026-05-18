#include "DetectionGateway.h"
#include "DetectionService.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

// Nest.js의 main.ts (bootstrap 함수) 에 대응
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // 1. 서비스 생성 (NestFactory + DI 컨테이너 역할)
    DetectionService detectionService;

    // 2. 게이트웨이 생성 (생성자 DI)
    DetectionGateway gateway(detectionService);

    // 3. uWebSockets 앱 생성 및 라우트 등록
    uWS::App app;
    gateway.registerRoutes(app);

    // 4. 포트 8081에서 리슨 시작
    app.listen(DetectionGateway::PORT, [](auto* listenSocket) {
        if (listenSocket) {
            std::cout << "[Server] Detection 서버 시작 — ws://localhost:"
                      << DetectionGateway::PORT << "/detection\n";
        } else {
            std::cerr << "[Server] 포트 바인딩 실패!\n";
        }
    }).run();   // 이벤트 루프 진입 (Node.js 이벤트 루프에 대응)

    return 0;
}
