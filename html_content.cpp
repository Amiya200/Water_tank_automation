#pragma once
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>Water Tank Controller</title>

<style>
body {
  font-family: 'Segoe UI', sans-serif;
  background: #ffffff;
  color: #0a192f;
  margin: 0;
  padding: 20px;
  text-align: center;
}

/* Tank */
.tank-level {
  width: 130px;
  height: 200px;
  border: 3px solid #00796b;
  border-radius: 12px;
  margin: 20px auto;
  position: relative;
  overflow: hidden;
}
.tank-fill {
  position: absolute;
  bottom: 0;
  width: 100%;
  background: linear-gradient(to top, #00c6ff, #0072ff);
  color: #fff;
  text-align: center;
  font-size: 18px;
  font-weight: bold;
  transition: height .5s;
}

/* Status */
.status {
  font-size: 18px;
  font-weight: 600;
  margin-bottom: 25px;
}
.on { color: #00a152; }
.off { color: #d32f2f; }

/* Buttons */
.button-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 12px;
  max-width: 340px;
  margin: 0 auto;
}
.button {
  padding: 14px;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  color: #fff;
  font-weight: 600;
  cursor: pointer;
  transition: 0.3s;
}

/* Colors */
.manual-off { background: #1e88e5; }
.manual-on { background: #0d47a1; }

.semi-off { background: #fbc02d; color: #000; }
.semi-on  { background: #f57f17; color: #000; }

/* ⭐ NEW – AUTO MODE COLORS */
.auto-off { background: #43a047; }     /* Green tone */
.auto-on  { background: #1b5e20; }      /* Darker active green */

.timer { background: #00acc1; color: #000; }
.countdown { background: #ef6c00; }
.twist { background: #00695c; }

</style>
</head>

<body>

<h2>Water Level</h2>
<div class="tank-level">
  <div id="levelFill" class="tank-fill" style="height:0%">0%</div>
</div>

<h2>Status</h2>
<div id="motorStatus" class="status off">Motor: OFF</div>
<div id="modeStatus" class="status">Mode: UNKNOWN</div>

<h2>Controls</h2>
<div class="button-grid">

  <!-- Toggle Manual -->
  <button id="manualBtn" class="button manual-off" onclick="toggleManual()">Manual Mode</button>

  <!-- ⭐ NEW – Toggle AUTO MODE -->
  <button id="autoBtn" class="button auto-off" onclick="toggleAuto()">Auto Mode</button>

  <!-- Toggle Semi-Auto -->
  <button id="semiBtn" class="button semi-off" onclick="toggleSemi()">Semi Auto</button>

  <!-- Other Pages -->
  <a class="button timer" href="/timer">Timer Mode</a>
  <a class="button countdown" href="/countdown">Countdown</a>
  <a class="button twist" href="/twist">Twist</a>

</div>

<script>
// LIVE STATUS POLLING
async function fetchStatus() {
  const res = await fetch("/status");
  const data = await res.json();

  // Motor Status
  const motorEl = document.getElementById("motorStatus");
  motorEl.innerHTML = "Motor: " + data.motor;
  motorEl.className = "status " + (data.motor === "ON" ? "on" : "off");

  // Mode Text
  document.getElementById("modeStatus").innerHTML = "Mode: " + data.mode;

  // Level Tank
  let lvl = data.level;
  document.getElementById("levelFill").style.height = lvl + "%";
  document.getElementById("levelFill").innerHTML = lvl + "%";

  updateButtons(data.mode);
}

// UPDATE UI COLORS BASED ON MODE
function updateButtons(mode) {

  const manualBtn = document.getElementById("manualBtn");
  const semiBtn   = document.getElementById("semiBtn");
  const autoBtn   = document.getElementById("autoBtn");

  // Manual Mode
  manualBtn.className = 
    (mode === "MANUAL") ? "button manual-on" : "button manual-off";

  // Semi Auto
  semiBtn.className =
    (mode === "SEMIAUTO") ? "button semi-on" : "button semi-off";

  // ⭐ AUTO MODE
  autoBtn.className =
    (mode === "AUTO") ? "button auto-on" : "button auto-off";
}

// MANUAL TOGGLE
async function toggleManual() {
  await fetch("/manual_toggle");
  fetchStatus();
}

// SEMI AUTO TOGGLE
async function toggleSemi() {
  await fetch("/semi_toggle");
  fetchStatus();
}

// ⭐ NEW – AUTO MODE TOGGLE
async function toggleAuto() {
  await fetch("/auto_toggle");
  fetchStatus();
}

setInterval(fetchStatus, 1000);
fetchStatus();
</script>

</body>
</html>
)rawliteral";
