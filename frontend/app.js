// app.js – Simple front‑end logic for the Hazard Animal Detection UI

window.mockDevices = [];
window.mockDetectionsLog = [];

window.initData = async function() {
  try {
    const resLogs = await fetch('http://localhost:8081/logs');
    window.mockDetectionsLog = await resLogs.json();
    
    const resDev = await fetch('http://localhost:8081/devices');
    window.mockDevices = await resDev.json();
    
    console.log("Successfully loaded data from backend.");
  } catch (err) {
    console.error("Failed to load data from backend (is the C++ server running on port 8081?):", err);
  }
};

window.getDeviceStats = function(deviceId) {
  const device = window.mockDevices.find(d => d.id === deviceId);
  if (!device) return null;
  
  const camIds = device.cameras.map(c => c.id);
  const deviceLogs = window.mockDetectionsLog.filter(l => camIds.includes(l.camera_id));
  
  const weeklyCounts = [0, 0, 0, 0, 0, 0, 0];
  if (deviceLogs.length > 0) {
    weeklyCounts[0] = deviceLogs.length; 
  }

  return {
    today: deviceLogs.length,
    week: deviceLogs.length * 4, 
    weeklyCounts: [4, 2, 5, 6, 7, 8, deviceLogs.length] 
  };
};

window.signalRaspberryPiSharedFile = function(ip, action, payload) {
  const timestamp = new Date().toLocaleTimeString();
  
  console.log(`[${timestamp}] 📡 [신호 전송] ${ip} 기기로 카메라 전원 제어 명령을 보냅니다... (${action})`);
  
  setTimeout(() => {
    const formattedPayload = JSON.stringify(payload, null, 2);
    console.log(`[${new Date().toLocaleTimeString()}] 💾 [공유 파일 수정] Pi 내의 sharedfile에 설정을 기록했습니다:\n${formattedPayload}`);
    
    setTimeout(() => {
      console.log(`[${new Date().toLocaleTimeString()}] 🟢 [동기화 완료] ${ip} 카메라 전원 동기화 완료.`);
    }, 800);
  }, 600);
};

/** Utility to create a status label */
function createStatusLabel(status) {
  const span = document.createElement('span');
  span.className = 'status-label';
  if (status === 'ready') {
    span.classList.add('status-ready');
    span.textContent = '🟢 Ready';
  } else if (status === 'offline') {
    span.classList.add('status-offline');
    span.textContent = '🔴 Offline';
  } else {
    span.textContent = status;
  }
  return span;
}

/** Render device cards on the dashboard */
function renderDeviceGrid() {
  const grid = document.getElementById('deviceGrid');
  grid.innerHTML = '';
  mockDevices.forEach(dev => {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.id = dev.id;
    card.innerHTML = `
      <div style="display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 1rem;">
        <h3 style="font-size: 1.2rem; font-weight: 600; color: #fff; margin: 0;">${dev.name || dev.ip}</h3>
        <button class="settings-cog" style="background: none; border: none; cursor: pointer; color: var(--color-text-secondary); transition: var(--transition-fast); padding: 4px; display: flex; align-items: center; justify-content: center;" title="설정 관리">
          <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="settings-svg">
            <circle cx="12" cy="12" r="3"></circle>
            <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"></path>
          </svg>
        </button>
      </div>
      <div style="display: flex; flex-direction: column; gap: 0.4rem; margin-bottom: 1.25rem;">
        <p style="margin: 0; display: flex; justify-content: space-between;"><span style="color: var(--color-text-secondary); font-size: 0.9rem;">IP 주소</span> <span style="font-weight: 500; font-size: 0.9rem; color: var(--color-text-primary);">${dev.ip}</span></p>
        <p style="margin: 0; display: flex; justify-content: space-between;"><span style="color: var(--color-text-secondary); font-size: 0.9rem;">OS 환경</span> <span style="font-weight: 500; font-size: 0.9rem; color: var(--color-text-primary);">${dev.os}</span></p>
        <p style="margin: 0; display: flex; justify-content: space-between;"><span style="color: var(--color-text-secondary); font-size: 0.9rem;">네트워크 상태</span> <span style="font-weight: 500; font-size: 0.9rem; color: var(--color-text-primary);">${dev.network}</span></p>
        <p style="margin: 0; display: flex; justify-content: space-between;"><span style="color: var(--color-text-secondary); font-size: 0.9rem;">연결된 카메라</span> <span style="font-weight: 600; font-size: 0.9rem; color: var(--color-primary-light);">${dev.cameras.length}대</span></p>
      </div>
    `;
    card.appendChild(createStatusLabel(dev.status));
    
    // Add CSS rotator styling dynamically for ease
    const cog = card.querySelector('.settings-cog');
    cog.addEventListener('mouseover', () => {
      cog.querySelector('.settings-svg').style.transform = 'rotate(45deg)';
      cog.querySelector('.settings-svg').style.color = '#fff';
    });
    cog.addEventListener('mouseout', () => {
      cog.querySelector('.settings-svg').style.transform = 'rotate(0deg)';
      cog.querySelector('.settings-svg').style.color = 'var(--color-text-secondary)';
    });

    card.addEventListener('click', () => {
      // Navigates when card is clicked
      location.href = `device.html?deviceId=${dev.id}`;
    });
    grid.appendChild(card);
  });
}

/** Modal handling for adding a new device */
function initAddDeviceModal() {
  const fab = document.getElementById('fabAdd');
  const overlay = document.getElementById('modalOverlay');
  const form = document.getElementById('addDeviceForm');
  const testBtn = document.getElementById('testConnection');

  fab.addEventListener('click', () => overlay.classList.add('active'));
  overlay.addEventListener('click', e => {
    if (e.target === overlay) overlay.classList.remove('active');
  });

  testBtn.addEventListener('click', () => {
    const ip = document.getElementById('deviceIP').value.trim();
    if (!ip) return alert('IP 주소를 입력하세요.');
    alert(`연결 테스트 중... (IP: ${ip})`);
  });

  form.addEventListener('submit', e => {
    e.preventDefault();
    const name = document.getElementById('deviceName').value.trim();
    const ip = document.getElementById('deviceIP').value.trim();
    const os = document.getElementById('deviceOS').value.trim();
    if (!ip) return alert('IP 주소는 필수 항목입니다.');
    const newDev = {
      id: Date.now(),
      name: name || ip,
      ip,
      status: 'ready',
      os: os || 'Ubuntu 22.04 LTS',
      network: '1 Gbps',
      cameras: []
    };
    mockDevices.push(newDev);
    renderDeviceGrid();
    overlay.classList.remove('active');
    form.reset();
  });
}

/** Initialize everything on page load */
window.addEventListener('DOMContentLoaded', async () => {
  await window.initData();
  if (document.getElementById('deviceGrid')) {
    renderDeviceGrid();
    initAddDeviceModal();
  }
});
