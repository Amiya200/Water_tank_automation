#pragma once
const char* countdownModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Countdown Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      margin: 0;
      padding: 0;
      font-family: 'Segoe UI', sans-serif;
      background: radial-gradient(circle at top, #0f2027, #203a43, #2c5364);
      color: #ffffff;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }

    .container {
      width: 90%;
      max-width: 420px;
      background: rgba(255, 255, 255, 0.05);
      padding: 28px;
      border-radius: 14px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.4);
      backdrop-filter: blur(8px);
    }

    h1 {
      text-align: center;
      color: #00bcd4;
      margin-bottom: 24px;
      font-weight: 600;
    }

    label { 
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
      color: #d1d5db;
    }

    input[type="number"] {
      width: 100%;
      padding: 12px;
      border: 1px solid #37474f;
      border-radius: 8px;
      margin-bottom: 16px;
      font-size: 15px;
      background-color: #1e272e;
      color: #ffffff;
      outline: none;
      transition: border-color 0.2s;
    }

    input[type="number"]:focus {
      border-color: #00bcd4;
    }

    .button {
      display: block;
      width: 100%;
      padding: 14px;
      font-size: 16px;
      border: none;
      border-radius: 10px;
      color: #ffffff;
      background: linear-gradient(135deg, #009688, #26c6da);
      cursor: pointer;
      transition: all 0.2s ease-in-out;
    }

    .button.stop {
      background: linear-gradient(135deg, #f44336, #e57373);
    }

    .button:hover { 
      opacity: 0.95;
      transform: scale(1.02);
    }

    #message, #countdownDisplay {
      margin-top: 16px;
      font-size: 16px;
      text-align: center;
    }

    #countdownDisplay {
      color: #00e676;
      font-weight: 600;
    }

    .back-btn {
      margin-top: 24px;
      background: linear-gradient(135deg, #1565c0, #42a5f5);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Countdown Mode</h1>

    <form id="countdownForm">
      <label>Duration (minutes)</label>
      <input type="number" id="duration" name="duration" min="1" max="180" required />
      <button type="button" id="toggleBtn" class="button" onclick="startCountdown()">Start</button>
    </form>

    <p id="countdownDisplay">Waiting for input...</p>
    <p id="message"></p>

    <a href="/"><button class="button back-btn" type="button">Back</button></a>
  </div>

  <script>
    let remaining = 0;
    let interval = null;

    async function startCountdown() {
      const mins = parseInt(document.getElementById("duration").value);
      if (!mins || mins < 1 || mins > 180) {
        alert("Please enter a valid duration between 1 and 180 minutes.");
        return;
      }

      try {
        const res = await fetch(`/start_countdown?duration=${mins}`);
        const msg = await res.text();
        document.getElementById("message").textContent = msg;
        remaining = mins * 60;
        startCountdownDisplay();
      } catch {
        document.getElementById("message").textContent = "Failed to communicate with device.";
      }
    }

    function startCountdownDisplay() {
      clearInterval(interval);
      interval = setInterval(() => {
        if (remaining <= 0) {
          clearInterval(interval);
          document.getElementById("countdownDisplay").textContent =
            "Countdown finished.";
          return;
        }
        const m = Math.floor(remaining / 60);
        const s = remaining % 60;
        document.getElementById("countdownDisplay").textContent =
          `Motor will toggle in ${m}:${s.toString().padStart(2,'0')}`;
        remaining--;
      }, 1000);
    }
  </script>
</body>
</html>
)rawliteral";
