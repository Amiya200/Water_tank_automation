#pragma once
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Water Tank Controller</title>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background-color: #f4ecf7; /* Sky Blue */
      color: #000;
      margin: 0;
      padding: 20px;
      text-align: center;
    }

    .logo {
      max-width: 140px;
      margin: 10px auto;
    }

    .tank-level {
      width: 100px;
      height: 150px;
      border: 2px solid #555;
      margin: 20px auto;
      border-radius: 8px;
      position: relative;
      overflow: hidden;
      background: #ffffff;
    }

    .tank-fill {
      position: absolute;
      bottom: 0;
      width: 100%;
      background-color: #00aaff;
      color: #fff;
      text-align: center;
      font-weight: bold;
      font-size: 18px;
    }

    .label {
      display: inline-block;
      margin: 5px 10px;
      padding: 5px 10px;
      border-radius: 5px;
      font-weight: bold;
    }

    .ground { background: #007bff; color: white; }
    .available { background: #28a745; color: white; }

    h2 {
      margin-top: 30px;
      color: #000;
    }

    .button-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      max-width: 300px;
      margin: 10px auto;
    }

    .button {
      padding: 12px;
      font-size: 16px;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      color: white;
    }

    .toggle { background-color: #ff5722; }
    .error { background-color: #b00020; }
    .timer { background-color: #0dcaf0; color: black; }
    .search { background-color: #6200ea; }
    .countdown { background-color: #fb8c00; }
    .twist { background-color: #009688; }
    .semi { background-color: #ffc107; color: black; }
    .manual { background-color: #1976d2; }

    a.settings {
      display: block;
      margin-top: 20px;
      text-decoration: none;
      color: #0d6efd;
      font-weight: bold;
    }

    a.button {
      text-decoration: none;
      text-align: center;
      line-height: 40px;
    }

    #motorMsg {
      margin-top: 10px;
      font-size: 16px;
      color: green;
    }

  </style>
</head>
<body>
  <img src="https://upload.wikimedia.org/wikipedia/commons/6/60/Halonix_logo.png" class="logo" alt="Halonix Logo"/>

  <div class="tank-level">
    <div id="fill" class="tank-fill" style="height: 70%;">70%</div>
  </div>

  <div>
    <span class="label ground">Ground Water</span>
    <span class="label available">Available</span>
  </div>

  <h2>Motor Control</h2>
  <div class="button-grid">
    <button class="button toggle" id="motorToggleBtn" onclick="toggleMotor()">Loading...</button>
    <button class="button error" onclick="location.href='/error_box'">Error Box</button>
  </div>

  <div class="button-grid">
    <a href="/timer" class="button timer">Timer Mode</a>
    <a href="/search" class="button search">Search Mode</a>
    <a href="/countdown" class="button countdown">Count Down</a>
    <a href="/twist" class="button twist">Twist Mode</a>
    <a href="/semi" class="button semi">Semi-Auto</a>
    <a href="/manual" class="button manual">Manual Mode</a>
  </div>

  <a href="/settings" class="settings">Settings</a>

  <p id="motorMsg"></p>

  <script>
    let motorIsOn = false;

    function updateTankLevel() {
      fetch('/status').then(res => res.json()).then(data => {
        const fill = document.getElementById('fill');
        fill.style.height = data.level + '%';
        fill.innerText = data.level + '%';
      });
    }

    function getMotorStatus() {
      fetch('/motor_status').then(res => res.text()).then(status => {
        motorIsOn = (status.trim() === "ON");
        document.getElementById("motorToggleBtn").innerText = motorIsOn ? "Turn OFF" : "Turn ON";
      });
    }

    function toggleMotor() {
      const endpoint = motorIsOn ? "/manual/off" : "/manual/on";
      fetch(endpoint)
        .then(res => res.text())
        .then(msg => {
          document.getElementById("motorMsg").innerText = "✅ " + msg;
          motorIsOn = !motorIsOn;
          document.getElementById("motorToggleBtn").innerText = motorIsOn ? "Turn OFF" : "Turn ON";
        });
    }

    setInterval(updateTankLevel, 5000);
    window.onload = () => {
      updateTankLevel();
      getMotorStatus();
    };
  </script>
</body>
</html>
)rawliteral";
