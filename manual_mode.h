#pragma once
const char* manualModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Manual Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background-color: #121212;
      color: white;
      font-family: 'Segoe UI', sans-serif;
      text-align: center;
      padding: 40px;
    }
    .button {
      padding: 15px 30px;
      margin: 10px;
      font-size: 18px;
      background: #0d6efd;
      border: none;
      color: white;
      border-radius: 8px;
      cursor: pointer;
      transition: 0.3s;
    }
    .button:hover {
      background-color: #3d8bfd;
    }
    .back-button {
      background-color: #0dcaf0;
      color: black;
      margin-top: 20px;
    }
    #status {
      margin-top: 20px;
      font-size: 18px;
      color: #ffd54f;
    }
    #message {
      margin-top: 10px;
      font-size: 16px;
      color: #00e676;
    }
  </style>
</head>
<body>
  <h1>Manual Motor Control</h1>

  <p id="status">Motor Status: <span id="motorState">Loading...</span></p>

  <button class="button" onclick="controlMotor(true)">Turn ON</button>
  <button class="button" onclick="controlMotor(false)">Turn OFF</button>

  <p id="message"></p>

  <a href="/"><button class="button back-button">⬅ Back</button></a>

  <script>
    async function updateMotorStatus() {
      try {
        const res = await fetch('/motor_status');
        const status = await res.text();
        document.getElementById("motorState").textContent = status;
      } catch (e) {
        document.getElementById("motorState").textContent = "Error";
      }
    }

    async function controlMotor(turnOn) {
      const endpoint = turnOn ? '/manual/on' : '/manual/off';
      const res = await fetch(endpoint);
      const result = await res.text();
      document.getElementById("message").textContent = "✅ " + result;
      updateMotorStatus();
    }

    window.onload = updateMotorStatus;
  </script>
</body>
</html>
)rawliteral";
