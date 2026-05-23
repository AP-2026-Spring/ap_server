// app.js – Simple front‑end logic for the Hazard Animal Detection UI

window.devices = [];
window.detectionsLog = [];

window.initData = async function() {
  try {
    const resLogs = await fetch('http://localhost:8081/logs');
    window.detectionsLog = await resLogs.json();
    
    const resDev = await fetch('http://localhost:8081/devices');
    window.devices = await resDev.json();
    
    console.log("Successfully loaded data from backend.");
  } catch (err) {
    console.error("Failed to load data from backend (is the C++ server running on port 8081?):", err);
  }
};

window.getDeviceStats = function(deviceId) {
  const device = window.devices.find(d => d.id === deviceId);
  if (!device) return null;
  
  const camIds = device.cameras.map(c => c.id);
  const deviceLogs = window.detectionsLog.filter(l => camIds.includes(l.camera_id));
  
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

window.signalRaspberryPiSharedFile = async function(ip, action, payload) {
  const timestamp = new Date().toLocaleTimeString();
  
  console.log(`[${timestamp}] 📡 [신호 전송] ${ip} 기기로 카메라 전원 제어 명령을 보냅니다... (${action})`);
  
  try {
    const response = await fetch('http://localhost:8081/camera/control', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        camera_id: payload.camera_id,
        action: action,
        enabled: payload.enabled,
        target_ip: ip
      })
    });
    
    const result = await response.json();
    
    if (result.mocked) {
      console.log(`[${new Date().toLocaleTimeString()}] 🟢 [테스트 모드] 인메모리에서 라즈베리파이 웹소켓 통신을 성공적으로 목킹했습니다.`);
    } else {
      console.log(`[${new Date().toLocaleTimeString()}] 🟢 [라이브 모드] 실제 라즈베리파이로 웹소켓 신호가 전송되었습니다.`);
    }
  } catch (err) {
    console.error("Failed to send camera control signal:", err);
  }
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
  window.devices.forEach(dev => {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.id = dev.id;
    card.innerHTML = `
      <div style="display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 1rem;">
        <h3 style="font-size: 1.2rem; font-weight: 600; color: #fff; margin: 0;">${dev.name || dev.ip}</h3>
        <button class="delete-card-btn" title="기기 삭제">
          <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <polyline points="3 6 5 6 21 6"></polyline>
            <path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path>
            <line x1="10" y1="11" x2="10" y2="17"></line>
            <line x1="14" y1="11" x2="14" y2="17"></line>
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
    
    const deleteBtn = card.querySelector('.delete-card-btn');

    // Delete device action
    deleteBtn.addEventListener('click', async (e) => {
      e.stopPropagation(); // Prevents card navigation to device.html

      const confirmDelete = confirm(`⚠️ 기기 삭제 경고\n\n정말로 이 기기를 삭제하시겠습니까?\n\n기기명: ${dev.name || dev.ip}\nIP 주소: ${dev.ip}`);
      if (!confirmDelete) return;

      try {
        const response = await fetch(`http://localhost:8081/devices/${dev.id}`, {
          method: 'DELETE'
        });
        
        if (response.ok) {
          const result = await response.json();
          if (result.success) {
            // Remove device from local state
            window.devices = window.devices.filter(d => d.id !== dev.id);
            // Premium scale and fade-out animation
            card.style.transition = 'transform 0.4s cubic-bezier(0.16, 1, 0.3, 1), opacity 0.4s ease';
            card.style.transform = 'scale(0.9) translateY(10px)';
            card.style.opacity = '0';
            setTimeout(() => {
              renderDeviceGrid();
            }, 400);
            console.log(`Device ID ${dev.id} successfully deleted.`);
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

    card.addEventListener('click', () => {
      // Navigates when card itself is clicked
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

  testBtn.addEventListener('click', async () => {
    const ip = document.getElementById('deviceIP').value.trim();
    if (!ip) return alert('IP 주소를 입력하세요.');
    
    const originalText = testBtn.textContent;
    testBtn.textContent = '⏱️ 테스트 중...';
    testBtn.disabled = true;
    
    try {
      const response = await fetch(`http://localhost:8081/devices/ping/${encodeURIComponent(ip)}`);
      const result = await response.json();
      
      if (result.connected) {
        alert(`🟢 연결 성공!\n\n라즈베리파이 기기(${ip})가 백엔드 서버와 웹소켓으로 정상 연결되어 있습니다.\n\n메시지: ${result.message}`);
      } else {
        alert(`🔴 연결 실패\n\n라즈베리파이 기기(${ip})를 찾을 수 없습니다.\n\n기기가 켜져 있고 웹소켓 연결이 활성화되어 있는지 확인해 주세요.\n(현재 백엔드는 오프라인 기기로 간주하여 가상/Mocking 모드로 테스트합니다.)`);
      }
    } catch (err) {
      console.error("Test connection failed:", err);
      alert(`⚠️ 서버 연결 오류\n\n백엔드 서버(port 8081)가 켜져 있는지 확인해 주세요.`);
    } finally {
      testBtn.textContent = originalText;
      testBtn.disabled = false;
    }
  });

  form.addEventListener('submit', async e => {
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
      cameras: [
        {
          id: 1001,
          name: "카메라 1 (CSI)",
          enabled: true,
          targets: { mouse: true, cockroach: true }
        },
        {
          id: 1002,
          name: "카메라 2 (USB)",
          enabled: true,
          targets: { mouse: true, cockroach: true }
        }
      ]
    };

    try {
      const response = await fetch('http://localhost:8081/devices', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(newDev)
      });
      
      if (response.ok) {
        window.devices.push(newDev);
        renderDeviceGrid();
        overlay.classList.remove('active');
        form.reset();
        console.log("Device added successfully to backend.");
      } else {
        alert("기기 추가에 실패했습니다.");
      }
    } catch (err) {
      console.error("Failed to post new device:", err);
      alert("서버 연결 실패. C++ 서버가 작동 중인지 확인하세요.");
    }
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
