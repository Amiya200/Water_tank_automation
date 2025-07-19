#pragma once
const char* semiAutoModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Semi Auto Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background-color: #000;
      color: white;
      font-family: 'Segoe UI', sans-serif;
      text-align: center;
      padding: 40px;
    }
    .button {
      padding: 15px 30px;
      font-size: 22px;
      background-color: #28a745; /* green by default */
      color: white;
      border: none;
      border-radius: 12px;
      cursor: pointer;
      margin: 20px auto;
      box-shadow: 0 4px 12px rgba(40, 167, 69, 0.4);
      transition: all 0.3s ease;
    }
    .button.stop {
      background-color: #dc3545;
      box-shadow: 0 4px 12px rgba(220, 53, 69, 0.4);
    }
    .button:hover {
      transform: scale(1.05);
    }
    #status {
      font-size: 20px;
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <h1>Semi Auto Control</h1>

  <button class="button" id="toggleBtn" onclick="toggleMotor()">Loading...</button>

  <p id="status">Motor Status: <span id="motorState">Loading...</span></p>

  <a href="/"><button class="button" style="background-color: #0dcaf0;">Back</button></a>

  <script>
    async function toggleMotor() {
      const res = await fetch('/semi_toggle');
      const data = await res.text();
      updateButton(data);
    }

    async function updateMotorState() {
      const res = await fetch('/motor_status');
      const data = await res.text();
      updateButton(data);
    }

    function updateButton(state) {
      const btn = document.getElementById('toggleBtn');
      const statusText = document.getElementById('motorState');

      statusText.innerText = state;

      if (state === "ON") {
        btn.innerText = "Stop Motor";
        btn.classList.add("stop");
      } else {
        btn.innerText = "Start Motor";
        btn.classList.remove("stop");
      }
    }

    window.onload = updateMotorState;
  </script>
</body>
</html>
)rawliteral";
