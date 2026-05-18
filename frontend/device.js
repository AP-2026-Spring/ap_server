// device.js – Logic for individual device management page

const mockDevices = window.mockDevices;

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
  
  if (device.cameras.length === 0) {
    container.innerHTML = `
      <div class="card" style="grid-column: 1 / -1; text-align: center; padding: 3rem; cursor: default;">
        <p style="color: var(--color-text-secondary); justify-content: center;">연결된 카메라가 없습니다.</p>
      </div>
    `;
    return;
  }

  device.cameras.forEach(cam => {
    // Calculate events dynamically to match backend consistency
    const camEvents = window.mockDetectionsLog ? window.mockDetectionsLog.filter(l => l.camera_id === cam.id).length : 0;

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

window.addEventListener('DOMContentLoaded', async () => {
  await window.initData();
  const devId = parseInt(getQueryParam('deviceId')) || 1;
  const device = mockDevices.find(d => d.id === devId);
  if (!device) {
    alert('기기를 찾을 수 없습니다.');
    return;
  }
  renderHeader(device);
  renderCameras(device);
  renderStats(device);
  initBackButton();
});
