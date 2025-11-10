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
    background: linear-gradient(180deg, #d4fcf9, #a6e5e1);
    color: #0a192f;
    margin: 0;
    padding: 20px;
    text-align: center;
  }

  .logo {
    max-width: 130px;
    margin: 10px auto 25px;
    display: block;
    filter: drop-shadow(0 0 4px rgba(0,0,0,0.25));
  }

  .tank-level {
    width: 130px;
    height: 200px;
    border: 3px solid #00796b;
    border-radius: 12px;
    margin: 25px auto 15px;
    position: relative;
    overflow: hidden;
    background: rgba(255,255,255,0.3);
    box-shadow: 0 0 20px rgba(0,121,107,0.25);
  }

  .tank-fill {
    position: absolute;
    bottom: 0;
    width: 100%;
    background: linear-gradient(to top, #00c6ff, #0072ff);
    color: #fff;
    text-align: center;
    font-weight: bold;
    font-size: 18px;
    transition: height 0.6s ease;
    box-shadow: inset 0 2px 6px rgba(0,0,0,0.3);
  }

  .status {
    margin-top: 12px;
    font-size: 18px;
    font-weight: 600;
  }

  .status.on { color: #00a152; text-shadow: 0 0 6px #00a152; }
  .status.off { color: #d32f2f; text-shadow: 0 0 6px #d32f2f; }

  .label {
    display: inline-block;
    margin: 10px 0;
    padding: 8px 18px;
    border-radius: 20px;
    font-weight: 600;
    color: #fff;
    background: #00796b;
    box-shadow: 0 0 10px rgba(0,121,107,0.4);
  }

  h2 {
    margin-top: 28px;
    font-weight: 600;
    color: #004d40;
    text-shadow: 0 0 5px rgba(0,0,0,0.1);
  }

  .button-grid {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 12px;
    max-width: 340px;
    margin: 15px auto 25px;
  }

  .button {
    padding: 12px;
    font-size: 16px;
    border: none;
    border-radius: 8px;
    color: #fff;
    font-weight: 500;
    background: linear-gradient(135deg, #0d47a1, #1976d2);
    cursor: pointer;
    transition: all 0.3s ease;
    box-shadow: 0 0 8px rgba(0,0,0,0.3);
  }

  .button:hover {
    transform: translateY(-2px);
    box-shadow: 0 0 12px rgba(0,121,107,0.4);
  }

  .timer { background: linear-gradient(135deg, #00acc1, #26c6da); color: #000; }
  .search { background: linear-gradient(135deg, #4527a0, #7e57c2); }
  .countdown { background: linear-gradient(135deg, #ef6c00, #ff9800); }
  .twist { background: linear-gradient(135deg, #00695c, #26a69a); }
  .semi { background: linear-gradient(135deg, #fbc02d, #fff176); color: #000; }
  .manual { background: linear-gradient(135deg, #1565c0, #42a5f5); }

  a.settings {
    display: block;
    margin-top: 20px;
    text-decoration: none;
    color: #004d40;
    font-weight: bold;
  }
</style>
</head>
<body>

  
  <img src="data:image/png;base64,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" class="logo" alt="Halonix Logo"/>


  <div class="tank-level">
    <div id="fill" class="tank-fill" style="height: 0%;">0%</div>
  </div>

  <div class="label" id="availabilityText">Checking water level...</div>

  <div class="status" id="motorStatus">Motor: Unknown | Mode: --</div>

  <h2>Control Modes</h2>
  <div class="button-grid">
    <a href="/timer" class="button timer">Timer Mode</a>
    <a href="/search" class="button search">Search Mode</a>
    <a href="/countdown" class="button countdown">Count Down</a>
    <a href="/twist" class="button twist">Twist Mode</a>
    <a href="/semi" class="button semi">Semi-Auto</a>
    <a href="/manual" class="button manual">Manual Mode</a>
  </div>

  <a href="/settings" class="settings">Settings</a>

  <script>
    async function updateDashboard() {
      try {
        const res = await fetch('/status');
        const data = await res.json();

        // Update tank fill
        const fill = document.getElementById('fill');
        fill.style.height = data.level + '%';
        fill.innerText = data.level + '%';

        // Update status
        const motorStatus = document.getElementById('motorStatus');
        motorStatus.textContent = "Motor: " + data.motor + " | Mode: " + data.mode;
        motorStatus.className = "status " + (data.motor === "ON" ? "on" : "off");

        // Update availability text
        const availText = document.getElementById('availabilityText');
        if (data.level > 10) {
          availText.textContent = "Ground Water Available";
          availText.style.background = "#1b5e20";
          availText.style.boxShadow = "0 0 12px rgba(27,94,32,0.7)";
        } else {
          availText.textContent = "Ground Water Not Available";
          availText.style.background = "#b71c1c";
          availText.style.boxShadow = "0 0 12px rgba(183,28,28,0.6)";
        }

      } catch (e) {
        document.getElementById('motorStatus').textContent = "Status: Offline";
      }
    }

    // Refresh every 3 seconds
    setInterval(updateDashboard, 3000);
    window.onload = updateDashboard;
  </script>
</body>
</html>
)rawliteral";
