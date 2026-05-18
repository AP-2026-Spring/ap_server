Write-Host "====================================================" -ForegroundColor Green
Write-Host "   Pest Detection - Automatic Network Setup Tool" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
Write-Host ""

# Ensure administrator privileges
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[ERROR] This script MUST be run as Administrator!" -ForegroundColor Red
    Write-Host "Please close this and open PowerShell as Administrator." -ForegroundColor Red
    Write-Host ""
    pause
    exit
}

# 1. Automatically retrieve WSL Internal IP
Write-Host "[1/3] Detecting WSL internal IP address..." -ForegroundColor Cyan
$wslIp = (wsl hostname -I).Trim().Split(" ")[0]
if (-not $wslIp) {
    Write-Host "[ERROR] Could not detect WSL IP. Is WSL running?" -ForegroundColor Red
    pause
    exit
}
Write-Host "Detected WSL IP: $wslIp" -ForegroundColor Yellow

# 2. Setup Netsh Portproxy
Write-Host "`n[2/3] Configuring port proxy (Port 8081: External -> WSL)..." -ForegroundColor Cyan
netsh interface portproxy delete v4tov4 listenport=8081 listenaddress=0.0.0.0 | Out-Null
netsh interface portproxy add v4tov4 listenport=8081 listenaddress=0.0.0.0 connectport=8081 connectaddress=$wslIp

# Verify
$checkProxy = netsh interface portproxy show all
if ($checkProxy -like "*8081*") {
    Write-Host "Portproxy successfully established!" -ForegroundColor Green
} else {
    Write-Host "[WARNING] Portproxy configuration might have failed. Please check manually." -ForegroundColor Yellow
}

# 3. Configure Windows Firewall Rule
Write-Host "`n[3/3] Setting up Windows Firewall rule for Port 8081..." -ForegroundColor Cyan
Remove-NetFirewallRule -DisplayName "Pest Detection WS Server" -ErrorAction SilentlyContinue | Out-Null
New-NetFirewallRule -DisplayName "Pest Detection WS Server" -Direction Inbound -LocalPort 8081 -Protocol TCP -Action Allow | Out-Null
Write-Host "Firewall rule successfully added/updated!" -ForegroundColor Green

# 4. Display Host LAN IP for Raspberry Pi .env
Write-Host "`n====================================================" -ForegroundColor Green
Write-Host "   Setup complete! Please configure Raspberry Pi:" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green

# Find active LAN/WIFI IP
$activeIp = (Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.IPAddress -notlike "127.*" -and $_.IPAddress -notlike "169.254.*" -and $_.IPAddress -notlike "172.*" }).IPAddress
Write-Host "Your Host PC IP Address: $activeIp" -ForegroundColor Yellow
Write-Host "Please set this IP in your Raspberry Pi's .env file:" -ForegroundColor Cyan
Write-Host "  WS_SERVER_IP=$activeIp" -ForegroundColor Cyan
Write-Host "  WS_SERVER_PORT=8081" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Green
Write-Host ""
pause
