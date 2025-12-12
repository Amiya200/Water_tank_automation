#ifndef SETTINGS_MODE_H
#define SETTINGS_MODE_H

const char* settingsModeHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Device Settings</title>
<style>
  body{font-family:Inter,Segoe UI,Arial;margin:0;background:#f4f7fb;color:#0b1220}
  .wrap{max-width:920px;margin:24px auto;padding:20px}
  .card{background:#fff;border-radius:12px;padding:18px;box-shadow:0 6px 18px rgba(11,18,32,0.06)}
  h1{margin:0 0 12px;font-size:20px}
  .row{display:flex;gap:12px;align-items:center;margin:8px 0;flex-wrap:wrap}
  .col{flex:1;min-width:140px}
  label{display:block;font-weight:600;margin-bottom:6px;font-size:13px;color:#223}
  input[type=number], input[type=text], input[type=time], select {width:100%;padding:8px;border-radius:8px;border:1px solid #e3e7ee}
  .small{font-size:12px;color:#556; margin-top:6px}
  .section{margin-top:14px;padding-top:12px;border-top:1px solid #eef2f7}
  .btn {background:#3b82f6;color:#fff;padding:10px 14px;border-radius:10px;border:0;font-weight:700;cursor:pointer}
  .muted{color:#667; font-size:13px}
  .flex-row{display:flex;gap:10px}
  .chip{background:#eef3ff;padding:6px 10px;border-radius:999px;font-size:13px;color:#1e40af}
  pre {background:#0b1220;color:#e6eefc;padding:10px;border-radius:8px;overflow:auto}
  .timer-slot {background:#fff;border-radius:8px;border:1px solid #e8eef8;padding:12px;margin-bottom:8px}
  .days {display:flex;gap:6px;flex-wrap:wrap}
  .days label {font-weight:500;font-size:13px;color:#334}
  .btn-small {padding:6px 10px;border-radius:8px;border:0;background:#ef4444;color:#fff;cursor:pointer}
  .error {border-color:#ef4444 !important; box-shadow:0 0 0 3px rgba(239,68,68,0.06);}
  .errtext {color:#ef4444;font-size:12px;margin-top:6px}
  .oktext {color:#10b981;font-size:12px;margin-top:6px}
  .disabled {opacity:0.6;pointer-events:none}
</style>
</head>
<body>
<div class="wrap">
  <div class="card">
    <h1>Device Settings</h1>
    <div class="small">Load, edit and apply settings. Fields here map to the keys sent to the STM32 device.</div>

    <!-- General -->
    <div class="section">
      <div class="row">
        <div class="col">
          <label>Device Name (1-32 chars)</label>
          <input id="deviceName" type="text" maxlength="32" placeholder="My Device" />
          <div id="err_deviceName" class="errtext" style="display:none">Name required (1-32 chars)</div>
        </div>
        <div class="col">
          <label>Operation Mode</label>
          <select id="opMode">
            <option value="AUTO">AUTO</option>
            <option value="MANUAL">MANUAL</option>
            <option value="SEMIAUTO">SEMIAUTO</option>
            <option value="TIMER">TIMER</option>
          </select>
          <div id="err_opMode" class="errtext" style="display:none">Invalid mode</div>
        </div>
        <div class="col">
          <label>Default Level (0-10)</label>
          <input id="defaultLevel" type="number" min="0" max="10" value="5" />
          <div id="err_defaultLevel" class="errtext" style="display:none">Must be 0–10</div>
        </div>
      </div>
    </div>

    <!-- Timer Mode (integrated: up to 5 slots w/ days + on/off) -->
    <div class="section">
      <div style="display:flex;justify-content:space-between;align-items:center">
        <div>
          <strong>Timer Mode (slots)</strong>
          <div class="small">Configure up to 5 timer slots. When a slot is enabled you must set days + ON + OFF.</div>
        </div>
        <div class="chip" id="timerSlotsBadge">0 slots</div>
      </div>

      <div id="timersContainer" style="margin-top:10px"></div>
      <div style="margin-top:10px;display:flex;gap:8px">
        <button class="btn" id="btnAddTimer" onclick="addTimerSlot()">+ Add Slot</button>
        <button style="background:#ef4444;color:#fff" onclick="clearAllTimers()">Clear Timers</button>
      </div>
      <div id="err_timers" class="errtext" style="display:none"></div>
    </div>

    <!-- Twist mode -->
    <div class="section">
      <h3 style="margin:6px 0">Twist Mode</h3>
      <div class="row">
        <div class="col">
          <label>Enabled</label>
          <select id="twistOn"><option value="1">Yes</option><option value="0">No</option></select>
        </div>
        <div class="col">
          <label>Twist ON duration (sec) — range 1–3600</label>
          <input id="twistOnDur" type="number" min="1" max="3600" value="30">
          <div id="err_twistOnDur" class="errtext" style="display:none">Value must be 1–3600</div>
        </div>
        <div class="col">
          <label>Twist OFF duration (sec) — range 1–3600</label>
          <input id="twistOffDur" type="number" min="1" max="3600" value="10">
          <div id="err_twistOffDur" class="errtext" style="display:none">Value must be 1–3600</div>
        </div>
      </div>
    </div>

    <!-- Sensors & thresholds -->
    <div class="section">
      <h3 style="margin:6px 0">Sensors & thresholds</h3>
      <div class="row">
        <div class="col">
          <label>Sensor threshold (water level) — 0–1023</label>
          <input id="sensorThreshold" type="number" min="0" max="1023" value="45">
          <div id="err_sensorThreshold" class="errtext" style="display:none">Must be 0–1023</div>
        </div>
        <div class="col">
          <label>Current alarm (A) — 0.0–100.0</label>
          <input id="currentAlarm" type="number" min="0" max="100" step="0.1" value="5.0">
          <div id="err_currentAlarm" class="errtext" style="display:none">Must be 0.0–100.0</div>
        </div>
        <div class="col">
          <label>Voltage alarm (V) — 0.0–300.0</label>
          <input id="voltageAlarm" type="number" min="0" max="300" step="0.1" value="200.0">
          <div id="err_voltageAlarm" class="errtext" style="display:none">Must be 0.0–300.0</div>
        </div>
      </div>
    </div>

    <!-- Relay labels / misc -->
    <div class="section">
      <h3 style="margin:6px 0">Relays / Labels</h3>
      <div class="row">
        <div class="col">
          <label>Relay 1 label (0-20 chars)</label>
          <input id="relay1Label" type="text" maxlength="20" value="Pump">
          <div id="err_relay1Label" class="errtext" style="display:none">0–20 chars</div>
        </div>
        <div class="col">
          <label>Relay 2 label (0-20 chars)</label>
          <input id="relay2Label" type="text" maxlength="20" value="Valve">
          <div id="err_relay2Label" class="errtext" style="display:none">0–20 chars</div>
        </div>
        <div class="col">
          <label>Relay 3 label (0-20 chars)</label>
          <input id="relay3Label" type="text" maxlength="20" value="Light">
          <div id="err_relay3Label" class="errtext" style="display:none">0–20 chars</div>
        </div>
      </div>
    </div>

    <!-- advanced raw settings -->
    <div class="section">
      <label>Raw extra settings (key=value;key2=val)</label>
      <input id="rawExtras" type="text" placeholder="e.g. debounceMs=50;maxRetries=3" maxlength="200">
      <div id="err_rawExtras" class="errtext" style="display:none">Too long (max 200 chars)</div>
      <div class="small">Use this to send any additional keys not shown above.</div>
    </div>

    <div style="margin-top:16px;display:flex;gap:10px;align-items:center">
      <button class="btn" id="applyBtn" onclick="applySettings()">Apply Settings</button>
      <button id="refreshBtn" onclick="refreshSettings()" style="padding:10px;border-radius:8px;border:1px solid #e3e7ee">Refresh</button>
      <div id="status" class="muted"></div>
    </div>

    <div style="margin-top:12px">
      <pre id="debugOut">Ready</pre>
    </div>

    <div style="height:10px"></div>
    <a href="/" class="small">Back to Home</a>
  </div>
</div>

<script>
const MAX_TIMER_SLOTS = 5;

function setError(id, msg) {
  const el = document.getElementById(id);
  if (!el) return;
  el.style.display = msg ? 'block' : 'none';
  el.innerText = msg || '';
  const input = el.previousElementSibling;
  if (input) {
    if (msg) input.classList.add('error'); else input.classList.remove('error');
  }
}

function mkId(id){ return document.getElementById(id); }

function makeTimerRow(idx, slot) {
  const el = document.createElement('div');
  el.className = 'timer-slot';
  el.id = 'timerRow_' + idx;

  const enabled = slot && slot.enabled ? 'checked' : '';
  const onVal = slot && slot.on ? slot.on : '';
  const offVal = slot && slot.off ? slot.off : '';
  const daysSet = {};
  if (slot && slot.days) {
    for (const d of slot.days) daysSet[d] = true;
  }

  el.innerHTML = `
    <div style="display:flex;justify-content:space-between;align-items:center">
      <h4 style="margin:0">Slot ${idx+1}</h4>
      <div>
        <label style="font-weight:600">Enabled</label>
        <input type="checkbox" id="timer_enable_${idx}" ${enabled}>
      </div>
    </div>

    <div style="margin-top:8px" class="days">
      <label><input type="checkbox" id="d${idx}_mon"> Mon</label>
      <label><input type="checkbox" id="d${idx}_tue"> Tue</label>
      <label><input type="checkbox" id="d${idx}_wed"> Wed</label>
      <label><input type="checkbox" id="d${idx}_thu"> Thu</label>
      <label><input type="checkbox" id="d${idx}_fri"> Fri</label>
      <label><input type="checkbox" id="d${idx}_sat"> Sat</label>
      <label><input type="checkbox" id="d${idx}_sun"> Sun</label>
    </div>

    <div style="display:flex;gap:12px;margin-top:8px">
      <div style="flex:1">
        <label>ON Time</label>
        <input id="timer_on_${idx}" type="time" value="${onVal}">
      </div>
      <div style="flex:1">
        <label>OFF Time</label>
        <input id="timer_off_${idx}" type="time" value="${offVal}">
      </div>
      <div style="width:68px;align-self:flex-end">
        <button class="btn-small" onclick="removeTimerSlot(${idx})">Del</button>
      </div>
    </div>
    <div id="err_timer_${idx}" class="errtext" style="display:none"></div>
  `;

  // set checkboxes after DOM insertion
  setTimeout(() => {
    const map = {mon:'mon', tue:'tue', wed:'wed', thu:'thu', fri:'fri', sat:'sat', sun:'sun'};
    for (const k in map) {
      const id = `d${idx}_${k}`;
      const elc = document.getElementById(id);
      if (!elc) continue;
      elc.checked = !!daysSet[map[k]];
    }
  }, 10);

  return el;
}

function renderTimers(slots) {
  const container = document.getElementById('timersContainer');
  container.innerHTML = '';
  for (let i=0;i<slots.length;i++){
    const node = makeTimerRow(i, slots[i]);
    container.appendChild(node);
  }
  updateTimerBadge();
  updateAddButtonState();
}

function updateTimerBadge() {
  const container = document.getElementById('timersContainer');
  const enabledCount = Array.from(container.children).reduce((acc, child, idx) => {
    const en = document.getElementById('timer_enable_'+idx);
    return acc + (en && en.checked ? 1 : 0);
  }, 0);
  document.getElementById('timerSlotsBadge').innerText = enabledCount + ' slots';
}

function updateAddButtonState() {
  const container = document.getElementById('timersContainer');
  const current = container.children.length;
  const btn = document.getElementById('btnAddTimer');
  if (current >= MAX_TIMER_SLOTS) btn.classList.add('disabled'); else btn.classList.remove('disabled');
}

function addTimerSlot() {
  const container = document.getElementById('timersContainer');
  const current = container.children.length;
  if (current >= MAX_TIMER_SLOTS) { alert('Maximum slots reached (' + MAX_TIMER_SLOTS + ')'); return; }
  const idx = current;
  const node = makeTimerRow(idx, {enabled:0, days:[], on:'', off:''});
  container.appendChild(node);
  updateTimerBadge();
  updateAddButtonState();
}

function removeTimerSlot(idx) {
  const el = document.getElementById('timerRow_' + idx);
  if (el) el.remove();
  // re-render to reindex ids (simple approach: rebuild from DOM)
  const container = document.getElementById('timersContainer');
  const slots = [];
  for (let i=0;i<container.children.length;i++){
    const row = container.children[i];
    // gather values before rebuild
    const en = row.querySelector(`#timer_enable_${i}`) ? row.querySelector(`#timer_enable_${i}`).checked : false;
    const on = row.querySelector(`#timer_on_${i}`) ? row.querySelector(`#timer_on_${i}`).value : '';
    const off = row.querySelector(`#timer_off_${i}`) ? row.querySelector(`#timer_off_${i}`).value : '';
    const days = [];
    const daysList = ['mon','tue','wed','thu','fri','sat','sun'];
    for (const d of daysList) {
      const elc = row.querySelector(`#d${i}_${d}`);
      if (elc && elc.checked) days.push(d);
    }
    slots.push({enabled: en ? 1 : 0, days, on, off});
  }
  renderTimers(slots);
}

function clearAllTimers() {
  document.getElementById('timersContainer').innerHTML = '';
  document.getElementById('timerSlotsBadge').innerText = '0 slots';
  updateAddButtonState();
}

function dbg(msg) { document.getElementById('debugOut').innerText = msg; }

async function refreshSettings() {
  dbg('Loading settings...');
  document.getElementById('status').innerText = 'Loading...';
  try {
    const res = await fetch('/settings/get');
    if(!res.ok) { const txt = await res.text(); dbg('Error '+res.status+': '+txt); document.getElementById('status').innerText='Error'; return; }
    const json = await res.json();
    // populate fields
    mkId('deviceName').value = json.deviceName || '';
    mkId('opMode').value = json.opMode || 'AUTO';
    mkId('defaultLevel').value = json.defaultLevel !== undefined ? json.defaultLevel : 5;
    mkId('twistOn').value = json.twistOn ? '1' : '0';
    mkId('twistOnDur').value = json.twistOnDur || 30;
    mkId('twistOffDur').value = json.twistOffDur || 10;
    mkId('sensorThreshold').value = json.sensorThreshold || 45;
    mkId('currentAlarm').value = json.currentAlarm || 5.0;
    mkId('voltageAlarm').value = json.voltageAlarm || 200.0;
    mkId('relay1Label').value = json.relay1Label || 'Pump';
    mkId('relay2Label').value = json.relay2Label || 'Valve';
    mkId('relay3Label').value = json.relay3Label || 'Light';
    mkId('rawExtras').value = json.rawExtras || '';

    // timers array (expected format: timers: [{enabled, days:[...], on, off}, ...])
    const timers = json.timers || [];
    const trimmed = timers.slice(0, MAX_TIMER_SLOTS);
    renderTimers(trimmed.length ? trimmed : []);
    document.getElementById('status').innerText = 'Loaded';
    dbg(JSON.stringify(json, null, 2));
  } catch (e) {
    dbg('Network error: ' + e.message);
    document.getElementById('status').innerText = 'Network error';
    console.error(e);
  }
}

// Build timers encoding:
// timers=idx:enabled,daysCSV,onTime,offTime|...
function buildSettingsKV() {
  // collect timer slots
  const container = document.getElementById('timersContainer');
  const slots = [];
  for (let i=0;i<container.children.length;i++){
    const enEl = document.getElementById('timer_enable_'+i);
    if (!enEl) continue;
    const enabled = enEl.checked ? 1 : 0;
    const on = document.getElementById('timer_on_'+i) ? document.getElementById('timer_on_'+i).value : '';
    const off = document.getElementById('timer_off_'+i) ? document.getElementById('timer_off_'+i).value : '';
    const days = [];
    const daysList = ['mon','tue','wed','thu','fri','sat','sun'];
    for (const d of daysList) {
      const el = document.getElementById(`d${i}_${d}`);
      if (el && el.checked) days.push(d);
    }
    slots.push({enabled, days, on, off});
  }

  // encode timers as timers=idx:enabled,daysCSV,on,off|...
  const timersEncodedParts = [];
  for (let i=0;i<slots.length;i++){
    const s = slots[i];
    const daysCSV = s.days.length ? s.days.join(',') : '';
    timersEncodedParts.push(`${i}:${s.enabled},${encodeURIComponent(daysCSV)},${encodeURIComponent(s.on)},${encodeURIComponent(s.off)}`);
  }
  const timersEncoded = timersEncodedParts.join('|');

  // Build key=value;key2=val string
  const parts = [];
  parts.push('deviceName=' + encodeURIComponent(document.getElementById('deviceName').value || ''));
  parts.push('opMode=' + encodeURIComponent(document.getElementById('opMode').value || 'AUTO'));
  parts.push('defaultLevel=' + encodeURIComponent(document.getElementById('defaultLevel').value || '5'));
  parts.push('twistOn=' + (document.getElementById('twistOn').value == '1' ? '1' : '0'));
  parts.push('twistOnDur=' + encodeURIComponent(document.getElementById('twistOnDur').value || '30'));
  parts.push('twistOffDur=' + encodeURIComponent(document.getElementById('twistOffDur').value || '10'));
  parts.push('sensorThreshold=' + encodeURIComponent(document.getElementById('sensorThreshold').value || '45'));
  parts.push('currentAlarm=' + encodeURIComponent(document.getElementById('currentAlarm').value || '5.0'));
  parts.push('voltageAlarm=' + encodeURIComponent(document.getElementById('voltageAlarm').value || '200.0'));
  parts.push('relay1Label=' + encodeURIComponent(document.getElementById('relay1Label').value || ''));
  parts.push('relay2Label=' + encodeURIComponent(document.getElementById('relay2Label').value || ''));
  parts.push('relay3Label=' + encodeURIComponent(document.getElementById('relay3Label').value || ''));
  if (timersEncoded) parts.push('timers=' + timersEncoded);
  const raw = document.getElementById('rawExtras').value || '';
  if (raw) parts.push(raw);

  return parts.join(';');
}

// Validation rules — returns true if valid
function validateSettings() {
  let ok = true;
  // deviceName 1-32
  const name = mkId('deviceName').value.trim();
  if (name.length < 1 || name.length > 32) {
    setError('err_deviceName', 'Name required (1-32 chars)');
    ok = false;
  } else setError('err_deviceName', '');

  // opMode
  const op = mkId('opMode').value;
  const allowedModes = ['AUTO','MANUAL','SEMIAUTO','TIMER'];
  if (!allowedModes.includes(op)) {
    setError('err_opMode', 'Invalid mode');
    ok = false;
  } else setError('err_opMode', '');

  // defaultLevel 0-10
  const dl = Number(mkId('defaultLevel').value);
  if (!Number.isFinite(dl) || dl < 0 || dl > 10) {
    setError('err_defaultLevel', 'Must be 0–10');
    ok = false;
  } else setError('err_defaultLevel', '');

  // twist durations 1-3600
  const tOn = Number(mkId('twistOnDur').value);
  const tOff = Number(mkId('twistOffDur').value);
  if (!Number.isFinite(tOn) || tOn < 1 || tOn > 3600) { setError('err_twistOnDur', 'Value must be 1–3600'); ok = false; } else setError('err_twistOnDur','');
  if (!Number.isFinite(tOff) || tOff < 1 || tOff > 3600) { setError('err_twistOffDur', 'Value must be 1–3600'); ok = false; } else setError('err_twistOffDur','');

  // sensorThreshold 0-1023
  const st = Number(mkId('sensorThreshold').value);
  if (!Number.isFinite(st) || st < 0 || st > 1023) { setError('err_sensorThreshold','Must be 0–1023'); ok = false; } else setError('err_sensorThreshold','');

  // currentAlarm 0.0-100.0
  const ca = Number(mkId('currentAlarm').value);
  if (!Number.isFinite(ca) || ca < 0 || ca > 100) { setError('err_currentAlarm','Must be 0.0–100.0'); ok = false; } else setError('err_currentAlarm','');

  // voltageAlarm 0.0-300.0
  const va = Number(mkId('voltageAlarm').value);
  if (!Number.isFinite(va) || va < 0 || va > 300) { setError('err_voltageAlarm','Must be 0.0–300.0'); ok = false; } else setError('err_voltageAlarm','');

  // relay labels 0-20 chars
  const r1 = mkId('relay1Label').value || '';
  const r2 = mkId('relay2Label').value || '';
  const r3 = mkId('relay3Label').value || '';
  if (r1.length > 20) { setError('err_relay1Label','0–20 chars'); ok = false; } else setError('err_relay1Label','');
  if (r2.length > 20) { setError('err_relay2Label','0–20 chars'); ok = false; } else setError('err_relay2Label','');
  if (r3.length > 20) { setError('err_relay3Label','0–20 chars'); ok = false; } else setError('err_relay3Label','');

  // rawExtras max length 200
  const raw = mkId('rawExtras').value || '';
  if (raw.length > 200) { setError('err_rawExtras','Too long (max 200 chars)'); ok = false; } else setError('err_rawExtras','');

  // timers validation
  const container = document.getElementById('timersContainer');
  const slots = [];
  for (let i=0;i<container.children.length;i++){
    const enEl = document.getElementById('timer_enable_'+i);
    if (!enEl) continue;
    const enabled = enEl.checked ? 1 : 0;
    const on = document.getElementById('timer_on_'+i) ? document.getElementById('timer_on_'+i).value : '';
    const off = document.getElementById('timer_off_'+i) ? document.getElementById('timer_off_'+i).value : '';
    const days = [];
    const daysList = ['mon','tue','wed','thu','fri','sat','sun'];
    for (const d of daysList) {
      const el = document.getElementById(`d${i}_${d}`);
      if (el && el.checked) days.push(d);
    }
    slots.push({enabled, days, on, off});
  }

  // enforce enabled slots must have at least one day and both on and off times
  let timerErrMsg = '';
  if (slots.length > MAX_TIMER_SLOTS) { timerErrMsg = `Max ${MAX_TIMER_SLOTS} timer slots allowed`; ok = false; }
  for (let i=0;i<slots.length;i++){
    const s = slots[i];
    const errEl = document.getElementById(`err_timer_${i}`);
    if (!errEl) continue;
    if (s.enabled) {
      if (!s.on || !s.off) {
        errEl.style.display = 'block';
        errEl.innerText = 'Enabled slot requires both ON and OFF times';
        ok = false;
      } else if (!s.days || s.days.length === 0) {
        errEl.style.display = 'block';
        errEl.innerText = 'Enabled slot requires at least one day';
        ok = false;
      } else {
        errEl.style.display = 'none';
        errEl.innerText = '';
      }
      // check time format HH:MM
      const timeRegex = /^[0-2]\d:[0-5]\d$/;
      if (s.on && !timeRegex.test(s.on)) { errEl.style.display='block'; errEl.innerText='ON time invalid'; ok = false; }
      if (s.off && !timeRegex.test(s.off)) { errEl.style.display='block'; errEl.innerText='OFF time invalid'; ok = false; }
      // optional: ensure on != off
      if (s.on && s.off && s.on === s.off) { errEl.style.display='block'; errEl.innerText='ON and OFF times should differ'; ok = false; }
    } else {
      errEl.style.display = 'none';
      errEl.innerText = '';
    }
  }
  setError('err_timers', timerErrMsg);

  // Update apply button state
  const applyBtn = document.getElementById('applyBtn');
  if (!ok) {
    applyBtn.classList.add('disabled');
  } else {
    applyBtn.classList.remove('disabled');
  }

  // update timer badge
  updateTimerBadge();

  return ok;
}

async function applySettings() {
  if (!validateSettings()) {
    dbg('Validation failed — fix errors before applying.');
    return;
  }
  const kv = buildSettingsKV();
  const dbgEl = document.getElementById('debugOut');
  dbgEl.innerText = 'Sending settings...\\n' + kv;
  document.getElementById('status').innerText = 'Sending...';
  try {
    // send to ESP endpoint
    const url = '/settings/set?data=' + encodeURIComponent(kv);
    const res = await fetch(url);
    const text = await res.text();
    if (res.ok) {
      document.getElementById('status').innerText = 'OK';
      dbgEl.innerText = 'STM32 replied:\\n' + text;
      // refresh after a short delay to allow device to update EEPROM/state
      setTimeout(refreshSettings, 800);
    } else {
      document.getElementById('status').innerText = 'Error';
      dbgEl.innerText = 'Error: ' + res.status + '\\n' + text;
    }
  } catch (e) {
    document.getElementById('status').innerText = 'Network error';
    dbgEl.innerText = 'Network error: ' + e.message;
  }
}

// real-time validation binding
document.addEventListener('input', (ev) => {
  // throttle: only validate top-level fields or timer inputs; simple approach: validate on input
  validateSettings();
});
document.addEventListener('change', (ev) => { validateSettings(); });

document.addEventListener('DOMContentLoaded', refreshSettings);
</script>
</body>
</html>
)rawliteral";

#endif // SETTINGS_MODE_H
