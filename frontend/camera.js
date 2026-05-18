// camera.js – Logic for individual camera detailed view page

function getQueryParam(name) {
  const params = new URLSearchParams(window.location.search);
  return params.get(name);
}

window.addEventListener('DOMContentLoaded', async () => {
  await window.initData();
  const devId = parseInt(getQueryParam('deviceId')) || 1;
  const camId = parseInt(getQueryParam('cameraId')) || 101;

  const device = window.mockDevices.find(d => d.id === devId);
  if (!device) {
    alert('기기를 찾을 수 없습니다.');
    return;
  }

  const camera = device.cameras.find(c => c.id === camId);
  if (!camera) {
    alert('카메라를 찾을 수 없습니다.');
    return;
  }

  // Set header info
  const headerDiv = document.getElementById('cameraHeader');
  headerDiv.innerHTML = `<h3 style="font-size: 1.25rem; font-weight: 700; color: #fff;">${camera.name} <span style="font-size: 0.9rem; font-weight: 400; color: var(--color-text-secondary);">(${device.name})</span></h3>`;

  // Set power toggle state
  const powerToggle = document.getElementById('cameraPower');
  powerToggle.checked = camera.enabled;

  // Get logs dynamically from the backend-aligned mock database
  const cameraLogs = window.mockDetectionsLog ? window.mockDetectionsLog.filter(l => l.camera_id === camera.id) : [];
  const latestLog = cameraLogs.length > 0 ? cameraLogs[0] : null;

  // Set snapshot image & info
  const snapshotImg = document.getElementById('snapshotImg');
  const snapshotInfo = document.getElementById('snapshotInfo');
  const recentEventText = document.getElementById('recentEvent');
  
  if (latestLog) {
    const classNameKr = latestLog.class_name === 'mouse' ? '쥐' : (latestLog.class_name === 'cockroach' ? '바퀴벌레' : latestLog.class_name);
    
    snapshotImg.src = latestLog.image_data || 'https://images.unsplash.com/photo-1557683316-973673baf926?w=600&auto=format&fit=crop&q=60';
    snapshotInfo.innerHTML = `<strong>감지 객체:</strong> ${classNameKr} (${Math.round(latestLog.confidence * 100)}% 신뢰도) <br> <span style="color: var(--color-text-secondary); font-size: 0.85rem;">타임스탬프: ${latestLog.timestamp}</span>`;
    
    recentEventText.textContent = `${classNameKr} 감지 (${Math.round(latestLog.confidence * 100)}% 신뢰도) - ${latestLog.timestamp}`;
  } else {
    // Placeholder image for empty camera
    snapshotImg.src = 'https://images.unsplash.com/photo-1557683316-973673baf926?w=600&auto=format&fit=crop&q=60';
    snapshotInfo.innerHTML = `이벤트가 감지되지 않았습니다.`;
    recentEventText.textContent = "없음";
  }

  // Detection target toggle setup
  const detectMouse = document.getElementById('detectMouse');
  const detectCockroach = document.getElementById('detectCockroach');

  detectMouse.checked = camera.targets.mouse;
  detectCockroach.checked = camera.targets.cockroach;

  function syncPiPowerConfig(action) {
    // Only manage camera power/usage through shared file as requested
    const payload = {
      camera_id: camera.id,
      enabled: camera.enabled,
      timestamp: Math.floor(Date.now() / 1000)
    };

    window.signalRaspberryPiSharedFile(device.ip, action, payload);
  }

  // Bind change events
  powerToggle.addEventListener('change', (e) => {
    camera.enabled = e.target.checked;
    syncPiPowerConfig(camera.enabled ? '카메라 사용 설정(ON)' : '카메라 사용 해제(OFF)');
  });

  detectMouse.addEventListener('change', (e) => {
    camera.targets.mouse = e.target.checked;
    console.log(`[로컬 설정] 쥐 탐지 필터가 ${e.target.checked ? '활성화' : '비활성화'} 되었습니다.`);
  });

  detectCockroach.addEventListener('change', (e) => {
    camera.targets.cockroach = e.target.checked;
    console.log(`[로컬 설정] 바퀴벌레 탐지 필터가 ${e.target.checked ? '활성화' : '비활성화'} 되었습니다.`);
  });

  // Render weekly stats chart (7 days)
  const chart = document.getElementById('weeklyChart');
  chart.innerHTML = '';
  const stats = window.getDeviceStats(device.id) || { weeklyCounts: [0, 0, 0, 0, 0, 0, 0] };
  const counts = stats.weeklyCounts;
  const max = Math.max(...counts, 1);
  const weekdays = ['월', '화', '수', '목', '금', '토', '일'];

  counts.forEach((cnt, idx) => {
    const col = document.createElement('div');
    col.style.display = 'flex';
    col.style.flexDirection = 'column';
    col.style.alignItems = 'center';
    col.style.flex = '1';
    col.style.height = '100%';
    col.style.justifyContent = 'flex-end';

    const bar = document.createElement('div');
    bar.className = 'bar';
    const heightPct = (cnt / max) * 80; // keep some space for labels
    bar.style.height = `${heightPct}%`;
    bar.style.width = '70%';
    bar.style.minHeight = cnt > 0 ? '4px' : '0px';
    bar.title = `${cnt}건`;

    const label = document.createElement('span');
    label.textContent = weekdays[idx];
    label.style.fontSize = '0.75rem';
    label.style.color = 'var(--color-text-secondary)';
    label.style.marginTop = '4px';

    col.appendChild(bar);
    col.appendChild(label);
    chart.appendChild(col);
  });

  // Back button handling
  const backBtn = document.getElementById('backBtn');
  backBtn.addEventListener('click', () => {
    // Go back to the specific device page
    location.href = `device.html?deviceId=${device.id}`;
  });
});
