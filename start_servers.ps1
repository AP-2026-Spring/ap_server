Write-Host "====================================================" -ForegroundColor Green
Write-Host "   Pest Detection System - Starting Servers (PowerShell)" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
Write-Host ""

$BASE_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path

# Resolving WSL paths
$DRIVE_LETTER = $BASE_DIR.Substring(0, 1).ToLower()
$PATH_WITHOUT_DRIVE = $BASE_DIR.Substring(2).Replace('\', '/')
$WSL_BUILD_DIR = "/mnt/$DRIVE_LETTER$PATH_WITHOUT_DRIVE/backend/build"
$WSL_EXE_PATH = "$WSL_BUILD_DIR/DetectionServer"

Write-Host "Host base directory: $BASE_DIR" -ForegroundColor Gray
Write-Host "Resolved WSL path  : $WSL_EXE_PATH`n" -ForegroundColor Gray

# 1. Build C++ Backend Server incrementally via WSL
Write-Host "====================================================" -ForegroundColor Green
Write-Host "   [1/3] Building C++ Backend Server (Incremental)..." -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
wsl bash -c "cd $WSL_BUILD_DIR && make"

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n====================================================" -ForegroundColor Red
    Write-Host "   [ERROR] C++ Backend compilation failed!" -ForegroundColor Red
    Write-Host "   Please fix compiler errors and try again." -ForegroundColor Red
    Write-Host "====================================================" -ForegroundColor Red
    Write-Host ""
    pause
    exit
}

Write-Host "`nC++ Backend built successfully!`n" -ForegroundColor Green

# 2. Start C++ Backend Server in WSL (new console window)
Write-Host "[2/3] Launching C++ Backend Server on port 8081 (WSL)..." -ForegroundColor Cyan
Start-Process wsl -ArgumentList "bash", "-c", "`"$WSL_EXE_PATH`"; read -p 'Server stopped. Press enter to close...'"

# 3. Start Node.js Frontend Web Server (new console window)
Write-Host "[3/3] Launching Frontend Web Server on port 3000..." -ForegroundColor Cyan
Start-Process cmd -ArgumentList "/c", "npx -y http-server `"$BASE_DIR\frontend`" -p 3000 --cors"

Write-Host "" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
Write-Host "   All servers have been launched successfully!" -ForegroundColor Green
Write-Host "   - Web Frontend Dashboard: http://localhost:3000" -ForegroundColor Yellow
Write-Host "   - C++ Backend Gateway   : http://localhost:8081" -ForegroundColor Yellow
Write-Host "====================================================" -ForegroundColor Green
Write-Host ""
Write-Host "Close the newly opened terminal windows to shut down the servers."
Write-Host ""
