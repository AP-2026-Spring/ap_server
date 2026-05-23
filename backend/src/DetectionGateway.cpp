#include "DetectionGateway.h"
#include <iostream>
#include <algorithm>

#include <random>

DetectionGateway::DetectionGateway(DetectionService& service)
    : detectionService_(service) {}

void DetectionGateway::registerRoutes(uWS::App& app) {

    // @WebSocketGateway(8081) + @SubscribeMessage('detection_signal') 에 대응
    // Socket.io의 네임스페이스 '/'에 '/detection' 경로로 매핑
    app.ws</*PerSocketData=*/int>("/detection", {

        // --- 연결 이벤트 ---
        .open = [this](auto* ws) {
            std::string_view remote_ip = ws->getRemoteAddressAsText();
            std::cout << "[Gateway] 클라이언트 연결됨: IP = " << remote_ip << "\n";
            clients_.push_back(ws);
        },

        // --- 메시지 수신: handleSignal() 에 대응 ---
        // Socket.io의 emit('detection_signal', data) 를
        // WebSocket 텍스트 메시지로 수신
        .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode) {
            if (opCode != uWS::OpCode::TEXT) return;

            try {
                // 수신한 JSON 파싱
                json data = json::parse(message);

                // 만약 시스템 리소스 상태 통계 데이터라면 다른 연결된 웹 브라우저 대시보드로 브로드캐스트 전달
                if (data.contains("type") && data["type"] == "resource_stats") {
                    std::string payload = data.dump();
                    for (auto* client : clients_) {
                        if (client != ws) {
                            client->send(payload, uWS::OpCode::TEXT);
                        }
                    }
                    return;
                }

                std::cout << "Received signal from client: " << data.dump() << "\n";

                // 서비스에 저장 (this->detectionService_.saveDetection(data))
                json saved = detectionService_.saveDetection(data);
                if(saved.empty()) {
                    // 저장 실패 (confidence < 0.7) 시 클라이언트에게 알림
                    json errResp = {{"received", false}, {"error", "Low confidence, data ignored"}};
                    ws->send(errResp.dump(), uWS::OpCode::TEXT);
                    return;
                }

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
        .close = [this](auto* ws, int code, std::string_view reason) {
            std::string_view remote_ip = ws->getRemoteAddressAsText();
            std::cout << "[Gateway] 클라이언트 연결 해제: IP = " << remote_ip << " (code=" << code << ")\n";
            auto it = std::find(clients_.begin(), clients_.end(), ws);
            if (it != clients_.end()) {
                clients_.erase(it);
            }
        }
        
    });

    // HTTP GET /logs → getAllLogs() 결과 반환
   // HTTP GET / → 대시보드 HTML 페이지
app.get("/", [](auto* res, auto* req) {
    res->writeHeader("Content-Type", "text/html; charset=utf-8")
       ->end(R"(
<!DOCTYPE html>
<html lang="ko">
<head>
<meta charset="UTF-8">
<title>탐지 대시보드</title>
<style>
* { box-sizing: border-box; margin: 0; padding: 0; font-family: sans-serif; }
body { background: #f5f5f5; padding: 24px; color: #222; }
h1 { font-size: 22px; font-weight: 500; margin-bottom: 4px; }
.sub { font-size: 13px; color: #888; margin-bottom: 24px; }
.cards { display: grid; grid-template-columns: repeat(3, 1fr); gap: 12px; margin-bottom: 24px; }
.card { background: #fff; border-radius: 10px; padding: 16px; border: 1px solid #e5e5e5; }
.card p { font-size: 13px; color: #888; margin-bottom: 6px; }
.card span { font-size: 26px; font-weight: 500; }
.panels { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 24px; }
.panel { background: #fff; border-radius: 10px; padding: 16px; border: 1px solid #e5e5e5; }
.panel h2 { font-size: 14px; font-weight: 500; margin-bottom: 12px; }
.bar-row { margin-bottom: 10px; }
.bar-label { display: flex; justify-content: space-between; font-size: 13px; margin-bottom: 4px; }
.bar-bg { height: 6px; background: #f0f0f0; border-radius: 99px; }
.bar-fill { height: 6px; background: #1D9E75; border-radius: 99px; }
.badge { display: inline-block; padding: 2px 8px; border-radius: 6px; font-size: 12px; }
.badge-red { background: #FCEBEB; color: #A32D2D; }
.badge-green { background: #E6FCEF; color: #15803D; }
.badge-blue { background: #E6F1FB; color: #185FA5; }
.badge-amber { background: #FAEEDA; color: #854F0B; }
.badge-gray { background: #f0f0f0; color: #555; }
table { width: 100%; border-collapse: collapse; font-size: 13px; }
th { text-align: left; padding: 8px; color: #888; font-weight: 500; border-bottom: 1px solid #eee; }
td { padding: 8px; border-bottom: 1px solid #eee; }
.log-panel { background: #fff; border-radius: 10px; padding: 16px; border: 1px solid #e5e5e5; }
.log-panel h2 { font-size: 14px; font-weight: 500; margin-bottom: 12px; }
.class-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 8px; }
.dot { width: 8px; height: 8px; border-radius: 50%; display: inline-block; margin-right: 8px; }
.count-badge { background: #f0f0f0; color: #555; padding: 2px 8px; border-radius: 6px; font-size: 13px; }
</style>
</head>
<body>
<h1>탐지 대시보드</h1>
<p class="sub">Edge AI 기반 해충 탐지 시스템</p>

<div class="cards">
  <div class="card"><p>총 탐지 수</p><span id="total">-</span></div>
  <div class="card"><p>활성 카메라</p><span id="cameras">-</span></div>
  <div class="card"><p>마지막 탐지</p><span id="latest" style="font-size:16px; line-height:2">-</span></div>
</div>

<div class="panels">
  <div class="panel">
    <h2>카메라별 탐지 수</h2>
    <div id="cam-bars"></div>
  </div>
  <div class="panel">
    <h2>탐지 종류</h2>
    <div id="class-list"></div>
  </div>
</div>

<div class="log-panel">
  <h2>탐지 로그</h2>
  <table>
    <thead><tr><th>ID</th><th>카메라</th><th>종류</th><th>신뢰도</th><th>시각</th></tr></thead>
    <tbody id="log-body"></tbody>
  </table>
</div>

<script>
const BADGE = { mouse:'badge-red', cockroach:'badge-green', rat:'badge-amber' };
const DOT = { mouse:'#E24B4A', cockroach:'#22C55E', rat:'#BA7517' };
const TRANSLATIONS = { mouse: '쥐', cockroach: '바퀴벌레', rat: '쥐' };

function badgeClass(cls) { return BADGE[cls] || 'badge-gray'; }
function dotColor(cls) { return DOT[cls] || '#888'; }
function translateClass(cls) { return TRANSLATIONS[cls] || cls; }

async function load() {
  const res = await fetch('/logs');
  const logs = await res.json();

  document.getElementById('total').textContent = logs.length + '건';

  const camCount = {};
  const classCount = {};
  logs.forEach(l => {
    camCount[l.camera_id] = (camCount[l.camera_id] || 0) + 1;
    classCount[l.class] = (classCount[l.class] || 0) + 1;
  });

  document.getElementById('cameras').textContent = Object.keys(camCount).length + '대';
  document.getElementById('latest').textContent = logs.length
    ? new Date(logs[logs.length-1].timestamp).toLocaleString('ko-KR')
    : '-';

  const maxCam = Math.max(...Object.values(camCount), 1);
  document.getElementById('cam-bars').innerHTML = Object.entries(camCount).map(([cam, cnt]) => `
    <div class="bar-row">
      <div class="bar-label"><span>${cam}</span><span>${cnt}건</span></div>
      <div class="bar-bg"><div class="bar-fill" style="width:${Math.round(cnt/maxCam*100)}%"></div></div>
    </div>`).join('');

  document.getElementById('class-list').innerHTML = Object.entries(classCount).map(([cls, cnt]) => `
    <div class="class-row">
      <span><span class="dot" style="background:${dotColor(cls)}"></span>${translateClass(cls)}</span>
      <span class="count-badge">${cnt}건</span>
    </div>`).join('');

  document.getElementById('log-body').innerHTML = [...logs].reverse().map(l => `
    <tr>
      <td>${l.id}</td>
      <td>${l.camera_id}</td>
      <td><span class="badge ${badgeClass(l.class)}">${translateClass(l.class)}</span></td>
      <td>${Math.round(l.confidence * 100)}%</td>
      <td>${new Date(l.timestamp).toLocaleString('ko-KR')}</td>
    </tr>`).join('');
}

load();
setInterval(load, 5000);
</script>
</body>
</html>
)");
});

// HTTP GET /logs → JSON 데이터
app.get("/logs", [this](auto* res, auto* req) {
    auto logs = detectionService_.getAllLogs();
    json response = json::array();
    for (const auto& log : logs) response.push_back(log);
    res->writeHeader("Content-Type", "application/json")
       ->writeHeader("Access-Control-Allow-Origin", "*")
       ->end(response.dump(2));
});

// HTTP GET /devices → JSON 데이터
app.get("/devices", [this](auto* res, auto* req) {
    res->writeHeader("Content-Type", "application/json")
       ->writeHeader("Access-Control-Allow-Origin", "*")
       ->end(detectionService_.getDevices().dump(2));
});

// CORS Preflight for DELETE /devices/:id
app.options("/devices/:id", [](auto* res, auto* req) {
    res->writeHeader("Access-Control-Allow-Origin", "*")
       ->writeHeader("Access-Control-Allow-Methods", "DELETE, OPTIONS")
       ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
       ->end();
});

// HTTP DELETE /devices/:id → 기기 삭제
app.del("/devices/:id", [this](auto* res, auto* req) {
    std::string id_str(req->getParameter(0));
    bool success = false;
    try {
        uint64_t device_id = std::stoull(id_str);
        success = detectionService_.removeDevice(device_id);
    } catch (const std::exception& e) {
        std::cerr << "[Gateway] Failed to parse device ID: " << id_str << ", error: " << e.what() << "\n";
    }

    json response = {{"success", success}};
    res->writeHeader("Content-Type", "application/json")
       ->writeHeader("Access-Control-Allow-Origin", "*")
       ->end(response.dump());
});

// CORS Preflight for POST /devices
app.options("/devices", [](auto* res, auto* req) {
    res->writeHeader("Access-Control-Allow-Origin", "*")
       ->writeHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
       ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
       ->end();
});

// HTTP POST /devices → JSON 데이터
app.post("/devices", [this](auto* res, auto* req) {
    res->onAborted([]() {});
    std::string* buffer = new std::string();
    res->onData([this, res, buffer](std::string_view data, bool isLast) {
        buffer->append(data.data(), data.length());
        if (isLast) {
            try {
                json newDevice = json::parse(*buffer);
                detectionService_.addDevice(newDevice);
                
                json response = {{"success", true}};
                res->writeHeader("Content-Type", "application/json")
                   ->writeHeader("Access-Control-Allow-Origin", "*")
                   ->end(response.dump());
            } catch (...) {
                res->writeStatus("400 Bad Request")
                   ->writeHeader("Access-Control-Allow-Origin", "*")
                   ->end("Invalid JSON");
            }
            delete buffer;
        }
});
});

// HTTP GET /devices/ping/:ip → 웹소켓 실시간 연결 여부 확인
app.get("/devices/ping/:ip", [this](auto* res, auto* req) {
    res->onAborted([]() {});
    
    std::string target_ip(req->getParameter(0));
    
    bool connected = false;
    for (auto* client : clients_) {
        std::string_view remote_view = client->getRemoteAddressAsText();
        std::string remote_ip(remote_view.data(), remote_view.size());
        
        // IP matching: exact match or localhost/127.0.0.1 equivalents
        bool ip_matches = (remote_ip == target_ip) ||
                          (target_ip == "127.0.0.1" && (remote_view == "::1" || remote_view == "localhost" || remote_view == "::ffff:127.0.0.1")) ||
                          (remote_ip == "127.0.0.1" && (target_ip == "::1" || target_ip == "localhost" || target_ip == "::ffff:127.0.0.1"));
        
        if (ip_matches) {
            connected = true;
            break;
        }
    }
    
    json response = {
        {"connected", connected},
        {"ip", target_ip},
        {"message", connected ? "Device is online (WebSocket connected)" : "Device is offline (No active WebSocket connection)"}
    };
    
    res->writeHeader("Content-Type", "application/json")
       ->writeHeader("Access-Control-Allow-Origin", "*")
       ->end(response.dump());
});


// CORS Preflight for POST /camera/control
app.options("/camera/control", [](auto* res, auto* req) {
    res->writeHeader("Access-Control-Allow-Origin", "*")
       ->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS")
       ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
       ->end();
});

// HTTP POST /camera/control → Mock or Real WebSocket to Raspberry Pi
app.post("/camera/control", [this](auto* res, auto* req) {
    // To handle async data, uWS requires us to buffer the data
    res->onAborted([]() { /* handle abort */ });
    
    std::string* buffer = new std::string();
    res->onData([this, res, buffer](std::string_view data, bool isLast) {
        buffer->append(data.data(), data.length());
        if (isLast) {
            try {
                json reqJson = json::parse(*buffer);
                
                int cam_id = reqJson.value("camera_id", 0);
                std::string raw_action = reqJson.value("action", "");
                bool enabled = (raw_action == "ON" || raw_action.find("ON") != std::string::npos || reqJson.value("enabled", false));
                std::string action = enabled ? "ON" : "OFF";
                std::string target_ip = reqJson.value("target_ip", "");
                
                bool sent_to_live = false;
                for (auto* client : clients_) {
                    std::string_view remote_view = client->getRemoteAddressAsText();
                    std::string remote_ip(remote_view.data(), remote_view.size());
                    
                    // IP matching: exact match or localhost/127.0.0.1 equivalents
                    bool ip_matches = (remote_ip == target_ip) ||
                                      (target_ip == "127.0.0.1" && (remote_ip == "::1" || remote_ip == "localhost" || remote_ip == "::ffff:127.0.0.1")) ||
                                      (remote_ip == "127.0.0.1" && (target_ip == "::1" || target_ip == "localhost" || target_ip == "::ffff:127.0.0.1"));
                    
                    if (ip_matches) {
                        std::cout << "[Live Mode] Sending " << action << " command (camera_id: " << cam_id << ") to device at " << remote_ip << "\n";
                        json payload = {
                            {"action", action},
                            {"camera_id", cam_id}
                        };
                        client->send(payload.dump(), uWS::OpCode::TEXT);
                        sent_to_live = true;
                    }
                }
                
                // Fallback: broadcast to all connected WebSocket clients if target_ip is empty or not matched
                if (!sent_to_live && !clients_.empty()) {
                    std::cout << "[Live Mode] No exact IP match found for target '" << target_ip << "'. Broadcasting " << action << " (camera_id: " << cam_id << ") to all connected devices.\n";
                    for (auto* client : clients_) {
                        json payload = {
                            {"action", action},
                            {"camera_id", cam_id}
                        };
                        client->send(payload.dump(), uWS::OpCode::TEXT);
                    }
                    sent_to_live = true;
                }

                // Always update the mock state in-memory so both UI and server-state remain in sync!
                detectionService_.updateMockCameraState(cam_id, enabled);

                json response;
                if (sent_to_live) {
                    response = {
                        {"success", true},
                        {"mocked", false},
                        {"message", "Sent to live Raspberry Pi client"}
                    };
                } else {
                    std::cout << "[Test Mode] No live WebSocket clients available. Falling back to Mocking (In-Memory).\n";
                    response = {
                        {"success", true},
                        {"mocked", true},
                        {"message", "Mocked Raspberry Pi WebSocket signal (No live clients)"}
                    };
                }

                res->writeHeader("Content-Type", "application/json")
                   ->writeHeader("Access-Control-Allow-Origin", "*")
                   ->end(response.dump());
            } catch (const std::exception& e) {
                res->writeStatus("400 Bad Request")
                   ->writeHeader("Access-Control-Allow-Origin", "*")
                   ->end("Invalid JSON");
            }
            delete buffer;
        }
    });
});

// CORS Preflight for POST /camera/targets
app.options("/camera/targets", [](auto* res, auto* req) {
    res->writeHeader("Access-Control-Allow-Origin", "*")
       ->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS")
       ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
       ->end();
});

// HTTP POST /camera/targets → Update rodent/insect filters
app.post("/camera/targets", [this](auto* res, auto* req) {
    res->onAborted([]() {});
    std::string* buffer = new std::string();
    res->onData([this, res, buffer](std::string_view data, bool isLast) {
        buffer->append(data.data(), data.length());
        if (isLast) {
            try {
                json reqJson = json::parse(*buffer);
                int cam_id = reqJson.value("camera_id", 0);
                bool mouse = reqJson.value("mouse", true);
                bool cockroach = reqJson.value("cockroach", true);
                
                detectionService_.updateCameraTargets(cam_id, mouse, cockroach);
                
                json response = {{"success", true}};
                res->writeHeader("Content-Type", "application/json")
                   ->writeHeader("Access-Control-Allow-Origin", "*")
                   ->end(response.dump());
            } catch (...) {
                res->writeStatus("400 Bad Request")
                   ->writeHeader("Access-Control-Allow-Origin", "*")
                   ->end("Invalid JSON");
            }
            delete buffer;
        }
    });
});
}
