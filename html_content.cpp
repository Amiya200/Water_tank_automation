
#include "html_content.h"

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
      background-color: #f4f8fb;
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
      border: 2px solid #aaa;
      margin: 20px auto;
      border-radius: 8px;
      position: relative;
      overflow: hidden;
      background: #fff;
    }
    .tank-fill {
      position: absolute;
      bottom: 0;
      width: 100%;
      background-color: #007bff;
      color: yellow;
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

    h2 { margin-top: 30px; }

    .button-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 10px;
      max-width: 500px;
      margin: 20px auto;
    }

    .button {
      padding: 12px;
      font-size: 16px;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      color: white;
    }

    .error { background-color: #dc3545; }
    .on { background-color: #198754; }
    .off { background-color: #6c757d; }
    .timer { background-color: #0dcaf0; }
    .search { background-color: #6610f2; }
    .countdown { background-color: #fd7e14; }
    .twist { background-color: #20c997; }
    .semi { background-color: #ffc107; color: black; }
    .manual { background-color: #0d6efd; }
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

  <h2>Control Panel</h2>
  <div class="button-grid">
    <button class="button on" onclick="fetch('/manual/on')">ON</button>
    <button class="button off" onclick="fetch('/manual/off')">OFF</button>
    <button class="button manual" onclick="location.href='/manual'">Manual Mode</button>
    <button class="button timer" onclick="location.href='/timer'">Timer</button>
    <button class="button countdown" onclick="location.href='/countdown'">Countdown</button>
    <button class="button search" onclick="location.href='/search'">Search</button>
    <button class="button twist" onclick="location.href='/twist'">Twist</button>
    <button class="button semi" onclick="location.href='/semi'">Semi-Auto</button>
    <button class="button error" onclick="location.href='/error_box'">Error Box</button>
  </div>

  <script>
    async function updateLevel(){
      try {
        const res = await fetch('/status');
        const data = await res.json();
        const fill = document.getElementById('fill');
        fill.style.height = data.level + '%';
        fill.textContent = data.level + '%';
      } catch (e) {
        console.error('Failed to fetch level:', e);
      }
    }
    setInterval(updateLevel, 5000);
    updateLevel();
  </script>
</body>
</html>
)rawliteral";
