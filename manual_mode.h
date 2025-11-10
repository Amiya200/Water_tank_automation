#pragma once
const char* manualModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Manual Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background-color: #0b0c10;
      color: #ffffff;
      font-family: 'Segoe UI', sans-serif;
      text-align: center;
      padding: 40px;
    }
    h1 {
      color: #00bcd4;
      margin-bottom: 30px;
    }
    .button {
      padding: 15px 30px;
      margin: 10px;
      font-size: 18px;
      background: #1f2833;
      border: 1px solid #45a29e;
      color: #ffffff;
      border-radius: 8px;
      cursor: pointer;
      transition: all 0.3s ease;
    }
    .button:hover {
      background-color: #45a29e;
      color: #0b0c10;
    }
    .back-button {
      background-color: #00bcd4;
      color: #0b0c10;
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


  <button class="button" onclick="controlMotor(true)">Turn ON</button>
  <button class="button" onclick="controlMotor(false)">Turn OFF</button>

  <p id="message"></p>

  <a href="/"><button class="button back-button">Back</button></a>

  <script>
    async function updateMotorStatus() {
      try {
        const res = await fetch('/motor_status');
        if (res.ok) {
          const status = await res.text();
          document.getElementById("motorState").textContent = status;
        } else {
          document.getElementById("motorState").textContent = "Unknown";
        }
      } catch (e) {
        document.getElementById("motorState").textContent = "Unavailable";
      }
    }

    async function controlMotor(turnOn) {
      const endpoint = turnOn ? '/manual/on' : '/manual/off';
      const res = await fetch(endpoint);
      const result = await res.text();
      document.getElementById("message").textContent = result;
      updateMotorStatus();
    }

    window.onload = updateMotorStatus;
  </script>
</body>
</html>
)rawliteral";
