#!/bin/bash

echo "===================================================="
echo "   Pest Detection System - Starting Servers (Linux) "
echo "===================================================="
echo ""

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

# 1. Build C++ Backend Server (Incremental)
echo "===================================================="
echo "  [1/3] Building C++ Backend Server (Incremental)..."
echo "===================================================="
cd "$BASE_DIR/backend/build"
make
if [ $? -ne 0 ]; then
    echo ""
    echo "===================================================="
    echo "  [ERROR] C++ Backend compilation failed!"
    echo "  Please fix compiler errors and try again."
    echo "===================================================="
    echo ""
    exit 1
fi
echo ""
echo "C++ Backend built successfully!"
echo ""

# 2. Start C++ Backend Server in background
echo "[2/3] Launching C++ Backend Server on port 8081..."
./DetectionServer &
BACKEND_PID=$!

# 3. Start Node.js Frontend Web Server in background
echo "[3/3] Launching Frontend Web Server on port 3000..."
cd "$BASE_DIR/frontend"
npx -y http-server -p 3000 --cors &
FRONTEND_PID=$!

echo ""
echo "===================================================="
echo "   All servers have been launched in the background!"
echo "   - Web Frontend Dashboard: http://localhost:3000"
echo "   - C++ Backend Gateway   : http://localhost:8081"
echo "===================================================="
echo ""
echo "Press Ctrl+C in this terminal to shut down both servers."
echo ""

# Handle shutdown gracefully
cleanup() {
    echo ""
    echo "Stopping servers..."
    kill $BACKEND_PID $FRONTEND_PID 2>/dev/null
    echo "Done. Goodbye!"
    exit 0
}

trap cleanup SIGINT SIGTERM

# Keep the shell script alive to monitor background jobs
wait
