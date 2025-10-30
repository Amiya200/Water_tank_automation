#pragma once
const char* countdownModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Countdown Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background-color: #eaecee;
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
    }
    .container {
      max-width: 420px;
      margin: 0 auto;
      background: #fff;
      padding: 20px;
      box-shadow: 0 4px 16px rgba(0,0,0,.12);
      border-radius: 14px;
      margin-top: 20px;
    }
    h1 { text-align: center; color: #2c3e50; margin-bottom: 14px; }
    label { display: block; margin-bottom: 6px; color: #2c3e50; }
    input[type="number"] {
      width: 100%;
      padding: 10px;
      border: 1px solid #ccd1d1;
      border-radius: 8px;
      margin-bottom: 14px;
      font-size: 14px;
    }
    .button {
      display: block;
      width: 100%;
      padding: 12px;
      font-size: 16px;
      background: #28a745;
      color: #fff;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      transition: .2s;
    }
    .button.stop { background: #dc3545; }
    .button:hover { opacity: .9; }
    #status, #message {
      margin-top: 12px;
      font-size: 14px;
      white-space: pre-wrap;
    }
    .back-btn {
      margin-top: 20px;
      background: #3498db;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Countdown</h1>
    <p id="status">Motor Status: <span id="motorText">Loading...</span></p>
    <form id="countdownForm">
      <label>Duration (minutes)</label>
      <input type="number" id="duration" name="duration" min="1" max="180" required />
      <button type="button" id="toggleBtn" class="button" onclick="startSmartCountdown()">Start</button>
    </form>
    <p id="message"></p>
    <a href="/"><button class="button back-btn" type="button">Back</button></a>
  </div>

  <script>
    let motorState = false;

    async function getMotorStatus() {
      try {
        const res = await fetch('/motor_status');
        const text = await res.text();
        motorState = (text.trim() === "ON");
        document.getElementById("motorText").textContent = motorState ? "ON" : "OFF";
        updateToggleLabel();
      } catch (e) {
        document.getElementById("motorText").textContent = "Unknown";
      }
    }

    function updateToggleLabel() {
      const btn = document.getElementById("toggleBtn");
      // If motor is currently ON, countdown will TURN IT OFF at the end
      // so we label as 'Start (will turn OFF)'
      btn.textContent = motorState ? "Start (Motor ON → OFF)" : "Start (Motor OFF → ON)";
      if (motorState) btn.classList.add("stop");
      else btn.classList.remove("stop");
    }

    // ✅ THIS was the broken part earlier (await in non-async)
    async function startSmartCountdown() {
      const mins = parseInt(document.getElementById("duration").value);
      if (!mins || mins < 1 || mins > 180) {
        alert("Enter 1-180 minutes");
        return;
      }
      const mode = motorState ? "on" : "off";
      try {
        const res = await fetch(`/start_countdown?duration=${mins}&mode=${mode}`);
        const msg = await res.text();
        document.getElementById("message").textContent = msg;
        // refresh motor status after sending
        setTimeout(getMotorStatus, 500);
      } catch (e) {
        document.getElementById("message").textContent = "Failed to send to ESP.";
      }
    }

    window.onload = getMotorStatus;
  </script>
</body>
</html>
)rawliteral";
