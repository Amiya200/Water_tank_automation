#pragma once
const char* countdownModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Countdown Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background: linear-gradient(180deg, #e3f2fd, #f1f8e9);
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
      color: #2c3e50;
    }
    .container {
      max-width: 420px;
      margin: 40px auto;
      background: #fff;
      padding: 22px;
      border-radius: 14px;
      box-shadow: 0 4px 16px rgba(0,0,0,.12);
    }
    h1 {
      text-align: center;
      color: #1565c0;
      margin-bottom: 18px;
    }
    label { 
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
    }
    input[type="number"] {
      width: 100%;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 8px;
      margin-bottom: 16px;
      font-size: 15px;
    }
    .button {
      display: block;
      width: 100%;
      padding: 12px;
      font-size: 16px;
      border: none;
      border-radius: 10px;
      color: #fff;
      background: linear-gradient(135deg, #43a047, #66bb6a);
      cursor: pointer;
      transition: 0.2s;
    }
    .button.stop {
      background: linear-gradient(135deg, #c62828, #ef5350);
    }
    .button:hover { opacity: 0.9; }
    #status, #message, #countdownDisplay {
      margin-top: 12px;
      font-size: 15px;
      white-space: pre-wrap;
      text-align: center;
    }
    #countdownDisplay {
      color: #0d47a1;
      font-weight: bold;
    }
    .back-btn {
      margin-top: 20px;
      background: linear-gradient(135deg, #1565c0, #42a5f5);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Countdown Mode</h1>
    <p id="status">Motor Status: <span id="motorText">Loading...</span></p>

    <form id="countdownForm">
      <label>Duration (minutes)</label>
      <input type="number" id="duration" name="duration" min="1" max="180" required />
      <button type="button" id="toggleBtn" class="button" onclick="startSmartCountdown()">Start</button>
    </form>

    <p id="countdownDisplay">Waiting for input</p>
    <p id="message"></p>

    <a href="/"><button class="button back-btn" type="button">Back</button></a>
  </div>

  <script>
    let motorState = false;
    let remaining = 0;
    let interval = null;

    async function getMotorStatus() {
      try {
        const res = await fetch('/motor_status');
        const text = await res.text();
        motorState = (text.trim() === "ON");
        document.getElementById("motorText").textContent = motorState ? "ON" : "OFF";
        updateToggleLabel();
      } catch {
        document.getElementById("motorText").textContent = "Unknown";
      }
    }

    function updateToggleLabel() {
      const btn = document.getElementById("toggleBtn");
      btn.textContent = motorState
        ? "Start (Motor ON → OFF)"
        : "Start (Motor OFF → ON)";
      if (motorState) btn.classList.add("stop");
      else btn.classList.remove("stop");
    }

    async function startSmartCountdown() {
      const mins = parseInt(document.getElementById("duration").value);
      if (!mins || mins < 1 || mins > 180) {
        alert("Enter 1–180 minutes");
        return;
      }

      const mode = motorState ? "on" : "off";
      try {
        const res = await fetch(`/start_countdown?duration=${mins}&mode=${mode}`);
        const msg = await res.text();
        document.getElementById("message").textContent = msg;
        remaining = mins * 60;
        startCountdownDisplay(mode);
        setTimeout(getMotorStatus, 800);
      } catch {
        document.getElementById("message").textContent = "Failed to send to ESP.";
      }
    }

    function startCountdownDisplay(mode) {
      clearInterval(interval);
      interval = setInterval(() => {
        if (remaining <= 0) {
          clearInterval(interval);
          document.getElementById("countdownDisplay").textContent =
            "Countdown finished! Motor will toggle now.";
          getMotorStatus();
          return;
        }
        const m = Math.floor(remaining / 60);
        const s = remaining % 60;
        document.getElementById("countdownDisplay").textContent =
          `${mode === "on" ? "Motor ON" : "Motor OFF"} – will switch in ${m}:${s.toString().padStart(2,'0')}`;
        remaining--;
      }, 1000);
    }

    window.onload = getMotorStatus;
  </script>
</body>
</html>
)rawliteral";
