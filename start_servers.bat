@echo off
title Pest Detection System Launcher
echo ====================================================
echo    Pest Detection System - Building and Starting...
echo ====================================================
echo.

:: Get workspace directory
set "BASE_DIR=%~dp0"
cd /d "%BASE_DIR%"

:: Extract drive letter and path
set "DRIVE=%~d0"
set "DRIVE_LETTER=%DRIVE:~0,1%"
set "DIR_PATH=%~p0"

:: Convert drive letter to lowercase for WSL /mnt/ mapping
set "DRIVE_LOWER=d"
for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
    if /i "%DRIVE_LETTER%"=="%%a" set "DRIVE_LOWER=%%a"
)

:: Replace backslashes with forward slashes for WSL compatibility
set "WSL_PATH_RAW=%DIR_PATH:\=/%"

:: Construct absolute WSL paths
set "WSL_BUILD_DIR=/mnt/%DRIVE_LOWER%%WSL_PATH_RAW%backend/build"
set "WSL_EXE_PATH=%WSL_BUILD_DIR%/DetectionServer"

echo Host base directory: %BASE_DIR%
echo Resolved WSL path  : %WSL_EXE_PATH%
echo.

:: 1. Build the C++ Backend Server inside WSL (Incremental Build)
echo ====================================================
echo   [1/3] Building C++ Backend Server (Incremental)...
echo ====================================================
wsl bash -c "cd %WSL_BUILD_DIR% && make"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ====================================================
    echo   [ERROR] C++ Backend compilation failed!
    echo   Please fix compiler errors and try again.
    echo ====================================================
    echo.
    pause
    exit /b %ERRORLEVEL%
)
echo.
echo C++ Backend built successfully!
echo.

:: 2. Start C++ Backend Server inside WSL in a new console window
echo [2/3] Launching C++ Backend Server on port 8081 (via WSL)...
start "Pest Detection C++ Backend" wsl bash -c "%WSL_EXE_PATH%; read -p 'Server stopped. Press enter to close...'"

:: 3. Start Node.js Frontend Web Server in a new console window
echo [3/3] Launching Frontend Web Server on port 3000 (via http-server)...
start "Pest Detection Web Frontend" cmd /c "npx -y http-server \"%BASE_DIR%frontend\" -p 3000 --cors"

echo.
echo ====================================================
echo    All servers have been launched successfully!
echo    - Web Frontend Dashboard: http://localhost:3000
echo    - C++ Backend Gateway   : http://localhost:8081
echo ====================================================
echo.
echo Close the newly opened terminal windows to shut down the servers.
echo.
pause
