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
      color: #e0e0e0;
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
    }

    .container {
      max-width: 420px;
      margin: 60px auto;
      padding: 30px 25px;
      background: #1c1c1c;
      border-radius: 16px;
      box-shadow: 0 0 18px rgba(63, 142, 252, 0.2);
    }

    h1 {
      text-align: center;
      color: #3f8efc;
      margin-bottom: 25px;
      font-size: 24px;
    }

    label {
      font-size: 16px;
      display: block;
      margin-bottom: 10px;
      text-align: left;
    }

    input[type="number"] {
      width: 100%;
      padding: 12px;
      font-size: 18px;
      background-color: #2b2b2b;
      border: none;
      border-radius: 8px;
      color: white;
      margin-bottom: 20px;
    }

    .button {
      width: 100%;
      padding: 14px;
      font-size: 18px;
      background-color: #3f8efc;
      border: none;
      border-radius: 10px;
      color: white;
      cursor: pointer;
      transition: 0.3s;
    }

    .button:hover {
      background-color: #64a7ff;
    }

    .back-btn {
      background-color: #5bc0de;
      color: black;
      margin-top: 15px;
    }

    .back-btn:hover {
      background-color: #7cd6ee;
    }

    #message, #status {
      margin-top: 20px;
      font-size: 16px;
      text-align: center;
    }

    #status {
      color: #ffd54f;
    }

    #message {
      color: #00e676;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Countdown Mode</h1>
    <form id="countdownForm">
      <label>Duration (in minutes)</label>
      <input type="number" id="duration" name="duration" min="1" max="180" required>

      <button type="button" class="button" id="toggleBtn" onclick="startSmartCountdown()">Start</button>
    </form>

    <p id="status">Motor Status: <span id="motorText">Loading...</span></p>
    <p id="message"></p>

    <a href="/"><button class="button back-btn">Back</button></a>
  </div>

  <script>
    let motorState = false;

    async function getMotorStatus() {
      try {
        const res = await fetch('/motor_status');
        const status = await res.text();
        motorState = (status.trim() === "ON");
        updateToggleLabel();
        document.getElementById("motorText").textContent = motorState ? "ON" : "OFF";
      } catch (e) {
        document.getElementById("motorText").textContent = "Unknown";
      }
    }

    function updateToggleLabel() {
      const btn = document.getElementById("toggleBtn");
      btn.textContent = motorState
        ? "Start Countdown to OFF"
        : "Start Countdown to ON";
    }

    async function startSmartCountdown() {
      const duration = parseInt(document.getElementById("duration").value);
      if (!duration || duration < 1 || duration > 180) {
        alert("Please enter a valid duration.");
        return;
      }

      const mode = motorState ? "on" : "off";
      const res = await fetch(`/start_countdown?duration=${duration}&mode=${mode}`);
      const msg = await res.text();

      document.getElementById("message").textContent = msg;
      document.getElementById("countdownForm").reset();

      motorState = !motorState; // expected future state
      updateToggleLabel();

      // Countdown: wait and fetch status again
      setTimeout(() => {
        getMotorStatus();
        document.getElementById("message").textContent = `Countdown finished — Motor is now ${motorState ? "ON" : "OFF"}`;
      }, duration * 60000); // convert minutes to milliseconds
    }

    window.onload = getMotorStatus;
  </script>
</body>
</html>
)rawliteral";
