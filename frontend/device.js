// device.js – Logic for individual device management page

/** Helper to read query param */
function getQueryParam(name) {
  const params = new URLSearchParams(window.location.search);
  return params.get(name);
}

/** Render the device header info */
function renderHeader(device) {
  const infoDiv = document.getElementById('deviceInfo');
  const isReady = device.status === 'ready';
  infoDiv.innerHTML = `
    <div style="display: flex; flex-direction: column;">
      <span style="font-size: 0.8rem; color: var(--color-text-secondary); text-transform: uppercase; letter-spacing: 0.5px;">기기 주소 / 이름</span>
      <h2 style="font-size: 1.25rem; font-weight: 700; color: #fff;">${device.name} <span style="font-weight: 400; color: var(--color-text-secondary); font-size: 1rem;">(${device.ip})</span></h2>
    </div>
    <span class="status-label ${isReady ? 'status-ready' : 'status-offline'}">
      ${isReady ? '🟢 Ready' : '🔴 Offline'}
    </span>
  `;
}

/** Render camera cards */
function renderCameras(device) {
  const container = document.getElementById('cameraList');
  container.innerHTML = '';
  
  if (!device.cameras || device.cameras.length === 0) {
    container.innerHTML = `
      <div class="card" style="grid-column: 1 / -1; text-align: center; padding: 3rem; cursor: default;">
        <p style="color: var(--color-text-secondary); justify-content: center;">연결된 카메라가 없습니다.</p>
      </div>
    `;
    return;
  }

  device.cameras.forEach(cam => {
    // Calculate events dynamically to match backend consistency
    const camEvents = window.detectionsLog ? window.detectionsLog.filter(l => l.camera_id === cam.id).length : 0;

    const card = document.createElement('div');
    card.className = 'card camera-card';
    card.dataset.id = cam.id;
    card.innerHTML = `
      <div class="camera-card-top">
        <h4 style="font-size: 1.15rem; font-weight: 600; color: #fff;">${cam.name}</h4>
        <label class="toggle" style="margin-top: -2px;">
          <input type="checkbox" ${cam.enabled ? 'checked' : ''} />
          <span class="slider"></span>
        </label>
      </div>
      <div class="camera-card-bottom">
        <span style="font-size: 0.85rem; color: var(--color-text-secondary);">이벤트 감지 활성화</span>
        <span style="font-size: 0.9rem; font-weight: 600; color: ${camEvents > 0 ? 'var(--color-danger)' : 'var(--color-text-secondary)'};">
          발견 ${camEvents}건
        </span>
      </div>
    `;

    // toggle handling
    const checkbox = card.querySelector('input');
    checkbox.addEventListener('change', e => {
      cam.enabled = e.target.checked;
      
      const action = cam.enabled ? '카메라 사용 설정(ON)' : '카메라 사용 해제(OFF)';
      // Only manage camera power/usage through shared file as requested
      const payload = {
        camera_id: cam.id,
        enabled: cam.enabled,
        timestamp: Math.floor(Date.now() / 1000)
      };

      window.signalRaspberryPiSharedFile(device.ip, action, payload);
    });

    // click to go to detailed camera page
    card.addEventListener('click', e => {
      // Don't trigger navigation if slider toggle or switch label is clicked
      if (!e.target.closest('.toggle')) {
        const url = `camera.html?deviceId=${device.id}&cameraId=${cam.id}`;
        location.href = url;
      }
    });

    container.appendChild(card);
  });
}

/** Render statistics panel */
function renderStats(device) {
  const stats = window.getDeviceStats(device.id) || { today: 0, week: 0, weeklyCounts: [0, 0, 0, 0, 0, 0, 0] };
  
  document.getElementById('todayCount').textContent = `${stats.today}건`;
  document.getElementById('weekCount').textContent = `${stats.week}건`;
  
  const chart = document.getElementById('weeklyChart');
  chart.innerHTML = '';
  const max = Math.max(...stats.weeklyCounts, 1);
  const weekdays = ['월', '화', '수', '목', '금', '토', '일'];

  stats.weeklyCounts.forEach((cnt, idx) => {
    const col = document.createElement('div');
    col.style.display = 'flex';
    col.style.flexDirection = 'column';
    col.style.alignItems = 'center';
    col.style.flex = '1';
    col.style.height = '100%';
    col.style.justifyContent = 'flex-end';

    const bar = document.createElement('div');
    bar.className = 'bar';
    const heightPct = (cnt / max) * 80; // scale to 80% to fit label
    bar.style.height = `${heightPct}%`;
    bar.style.width = '60%';
    bar.style.minHeight = cnt > 0 ? '4px' : '0px';
    bar.title = `${cnt}건`;

    const label = document.createElement('span');
    label.textContent = weekdays[idx];
    label.style.fontSize = '0.75rem';
    label.style.color = 'var(--color-text-secondary)';
    label.style.marginTop = '6px';

    col.appendChild(bar);
    col.appendChild(label);
    chart.appendChild(col);
  });
}

/** Back button handling */
function initBackButton() {
  const backBtn = document.getElementById('backBtn');
  backBtn.addEventListener('click', () => {
    location.href = 'index.html';
  });
}

/** Delete button handling */
function initDeleteButton(device) {
  const deleteBtn = document.getElementById('deleteDeviceBtn');
  if (!deleteBtn) return;
  
  deleteBtn.addEventListener('click', async () => {
    const confirmDelete = confirm(`⚠️ 기기 삭제 경고\n\n정말로 이 기기를 삭제하시겠습니까?\n\n기기명: ${device.name || device.ip}\nIP 주소: ${device.ip}`);
    if (!confirmDelete) return;

    try {
      const response = await fetch(`http://localhost:8081/devices/${device.id}`, {
        method: 'DELETE'
      });
      
      if (response.ok) {
        const result = await response.json();
        if (result.success) {
          alert('기기가 성공적으로 삭제되었습니다.');
          location.href = 'index.html';
        } else {
          alert('기기 삭제에 실패했습니다. (서버 처리 오류)');
        }
      } else {
        alert('기기 삭제 중 통신 에러가 발생했습니다.');
      }
    } catch (err) {
      console.error("Failed to delete device:", err);
      alert('서버 연결 실패. 백엔드 서버가 작동 중인지 확인하세요.');
    }
  });
}

let charts = {};

/** Initialize Chart.js performance graphs in a premium HSL custom themed layout */
function initCharts() {
  const chartConfigs = {
    cpu: { id: 'cpuChart', label: 'CPU 사용량 (%)', color: '#60a5fa', gradientColor: 'rgba(59, 130, 246, 0.2)', min: 0, max: 100 },
    ram: { id: 'ramChart', label: 'RAM 사용률 (%)', color: '#c084fc', gradientColor: 'rgba(168, 85, 247, 0.2)', min: 0, max: 100 },
    power: { id: 'powerChart', label: '소비 전력 (W)', color: '#facc15', gradientColor: 'rgba(234, 179, 8, 0.2)', min: 0, max: 10 },
    temp: { id: 'tempChart', label: '코어 온도 (°C)', color: '#fb7185', gradientColor: 'rgba(244, 63, 94, 0.2)', min: 20, max: 80 }
  };

  // Generate 300 initial labels representing 10 minutes rolling history (2-second interval)
  const initialLabels = Array.from({ length: 300 }, (_, i) => {
    const minAgo = 10 - Math.round((i / 299) * 10);
    return minAgo === 0 ? '현재' : `-${minAgo}분`;
  });

  const initialData = Array.from({ length: 300 }, () => null);

  Object.entries(chartConfigs).forEach(([key, conf]) => {
    const el = document.getElementById(conf.id);
    if (!el) return;
    const ctx = el.getContext('2d');
    
    // Create HSL glow line gradient
    const gradient = ctx.createLinearGradient(0, 0, 0, 130);
    gradient.addColorStop(0, conf.gradientColor);
    gradient.addColorStop(1, 'rgba(255, 255, 255, 0)');

    charts[key] = new Chart(ctx, {
      type: 'line',
      data: {
        labels: initialLabels,
        datasets: [{
          label: conf.label,
          data: [...initialData],
          borderColor: conf.color,
          borderWidth: 1.5,
          tension: 0.4,
          fill: true,
          backgroundColor: gradient,
          pointRadius: 0,
          pointHoverRadius: 4,
          pointBackgroundColor: conf.color,
          pointBorderColor: '#fff',
          pointBorderWidth: 1
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false, // Performance optimize for fast real-time ticks
        plugins: {
          legend: { display: false },
          tooltip: {
            mode: 'index',
            intersect: false,
            backgroundColor: 'rgba(15, 23, 42, 0.95)',
            titleColor: '#fff',
            bodyColor: '#fff',
            borderColor: 'rgba(255,255,255,0.08)',
            borderWidth: 1
          }
        },
        scales: {
          x: {
            grid: { color: 'rgba(255, 255, 255, 0.03)', drawTicks: false },
            ticks: {
              color: '#64748b',
              font: { size: 9 },
              maxRotation: 0,
              maxTicksLimit: 11, // clean spacing for exactly 10 minutes interval marks
              callback: function(val, index) {
                // Display tick every 30 points (1 minute)
                if (index % 30 === 0) {
                  const minAgo = 10 - Math.round(index / 30);
                  return minAgo === 0 ? '현재' : `-${minAgo}분`;
                }
                return '';
              }
            }
          },
          y: {
            min: conf.min,
            max: conf.max,
            grid: { color: 'rgba(255, 255, 255, 0.05)' },
            ticks: {
              color: '#64748b',
              font: { size: 9 },
              stepSize: (conf.max - conf.min) / 4
            }
          }
        }
      }
    });
  });
}

/** Update rolling 10 minutes timeline datasets for the Chart.js instance */
function updateChartData(chartKey, val) {
  const chart = charts[chartKey];
  if (!chart) return;
  
  const dataset = chart.data.datasets[0];
  dataset.data.push(val);
  if (dataset.data.length > 300) {
    dataset.data.shift();
  }
  
  // Use high-performance 'none' mode for low-overhead updates
  chart.update('none');
}

/** Establish WebSocket connection to receive real-time system metrics from the C++ backend */
function startResourceMonitor(device) {
  // Initialize Chart.js elements on first load
  if (Object.keys(charts).length === 0) {
    initCharts();
  }

  const wsUrl = "ws://localhost:8081/detection";
  console.log(`📡 Connecting to WebSocket system resource stream: ${wsUrl}`);
  
  const socket = new WebSocket(wsUrl);
  
  socket.onopen = () => {
    console.log("🟢 WebSocket system resource stream connected.");
  };
  
  socket.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      
      // Update DOM and Charts when receiving resource stats from the Edge C++ client thread
      if (data.type === "resource_stats") {
        const cpuVal = data.cpu;
        const ramVal = data.ram_gb;
        const ramMax = 4.0;
        const ramPct = Math.round((ramVal / ramMax) * 100);
        const powerVal = data.power;
        const tempVal = data.temp;

        // DOM Updates
        const cpuValEl = document.getElementById('cpuValue');
        if (cpuValEl) {
          cpuValEl.textContent = `${cpuVal}%`;
        }

        const ramValEl = document.getElementById('ramValue');
        const ramPctEl = document.getElementById('ramPercent');
        if (ramValEl && ramPctEl) {
          ramValEl.textContent = `${ramVal.toFixed(2)} GB / ${ramMax.toFixed(1)} GB`;
          ramPctEl.textContent = `${ramPct}% 사용 중`;
        }

        const powerValEl = document.getElementById('powerValue');
        const powerStatusEl = document.getElementById('powerStatus');
        if (powerValEl && powerStatusEl) {
          powerValEl.textContent = `${powerVal.toFixed(2)} W`;
          if (powerVal > 4.5) {
            powerStatusEl.textContent = 'High Load / Normal Mode';
          } else {
            powerStatusEl.textContent = 'Active / Low Power Mode';
          }
        }

        const tempValEl = document.getElementById('tempValue');
        const tempStatusEl = document.getElementById('tempStatus');
        if (tempValEl && tempStatusEl) {
          tempValEl.textContent = `${tempVal.toFixed(1)} °C`;
          if (tempVal > 70.0) {
            tempStatusEl.textContent = '🔥 경고: 과열 위험';
            tempStatusEl.style.color = '#f43f5e';
          } else if (tempVal > 55.0) {
            tempStatusEl.textContent = '⚠️ 주의: 온도 높음';
            tempStatusEl.style.color = '#eab308';
          } else {
            tempStatusEl.textContent = '🟢 적정 상태';
            tempStatusEl.style.color = 'var(--color-text-secondary)';
          }
        }

        // Live Performance Chart Updates (Rolling 10m Timeline)
        updateChartData('cpu', cpuVal);
        updateChartData('ram', ramPct);
        updateChartData('power', powerVal);
        updateChartData('temp', tempVal);
      }
    } catch (err) {
      console.error("Failed to parse WebSocket message:", err);
    }
  };
  
  socket.onclose = () => {
    console.log("🔴 WebSocket system resource stream closed. Reconnecting in 5s...");
    setTimeout(() => startResourceMonitor(device), 5000);
  };
  
  socket.onerror = (error) => {
    console.error("❌ WebSocket error:", error);
  };

  // Close previous socket if it exists on window
  if (window.resourceSocket) {
    window.resourceSocket.onclose = null;
    window.resourceSocket.close();
  }
  window.resourceSocket = socket;
}

window.addEventListener('DOMContentLoaded', async () => {
  await window.initData();
  const devId = parseInt(getQueryParam('deviceId')) || 1;
  const device = window.devices.find(d => d.id === devId);
  if (!device) {
    alert('기기를 찾을 수 없습니다.');
    return;
  }
  
  renderHeader(device);
  renderCameras(device);
  renderStats(device);
  initBackButton();
  initDeleteButton(device);
  startResourceMonitor(device);
});
