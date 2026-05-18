#pragma once
#include "DetectionService.h"
#include "uwebsockets/App.h"
#include <vector>

// detection.gateway.ts의 DetectionGateway 클래스에 대응
// @WebSocketGateway(8081) 에 대응 → PORT = 8081
class DetectionGateway {
public:
  static constexpr int PORT = 8081;

  // DetectionService를 주입받아 저장 (@Injectable DI 에 대응)
  explicit DetectionGateway(DetectionService &service);

  // uWebSockets 앱을 받아서 WebSocket 라우트를 등록
  // @SubscribeMessage('detection_signal') 등록에 대응
  void registerRoutes(uWS::App &app);

private:
  DetectionService &detectionService_;
  std::vector<uWS::WebSocket<false, true, int> *> clients_;
};
