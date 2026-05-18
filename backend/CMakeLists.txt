cmake_minimum_required(VERSION 3.20)
project(DetectionServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── 의존성: vcpkg로 설치 ───────────────────────────────────────────────────
# 설치 명령어:
#   vcpkg install uwebsockets nlohmann-json openssl zlib
#
# Visual Studio에서 vcpkg 통합:
#   vcpkg integrate install
# ──────────────────────────────────────────────────────────────────────────────

find_package(unofficial-uwebsockets CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# 소스 파일 목록
add_executable(DetectionServer
    src/main.cpp
    src/DetectionService.cpp
    src/DetectionGateway.cpp
    src/DatabaseManager.cpp
)

# 헤더 경로
target_include_directories(DetectionServer PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 라이브러리 링크
target_link_libraries(DetectionServer PRIVATE
    unofficial::uwebsockets::uwebsockets
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    unofficial::sqlite3::sqlite3
)

# Windows에서 Winsock 링크
if(WIN32)
    target_link_libraries(DetectionServer PRIVATE ws2_32)
endif()

# MSVC에서 소스 파일을 UTF-8로 해석하도록 지정 (한글 주석 등 멀티바이트 문자 대응)
if(MSVC)
    target_compile_options(DetectionServer PRIVATE /utf-8)
endif()

# ─── 테스트코드 ─────────────────────────────────────────────────────────────
add_executable(TestDatabase
    tests/test_database.cpp
    src/DatabaseManager.cpp
)

target_include_directories(TestDatabase PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(TestDatabase PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::sqlite3::sqlite3
)

if(MSVC)
    target_compile_options(TestDatabase PRIVATE /utf-8)
endif()
