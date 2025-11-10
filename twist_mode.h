#pragma once
const char* twistModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Twist Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background: radial-gradient(circle at top, #0f2027, #203a43, #2c5364);
      color: #fff;
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }

    .container {
      width: 90%;
      max-width: 420px;
      background: rgba(255,255,255,0.05);
      padding: 24px;
      border-radius: 14px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.4);
      backdrop-filter: blur(8px);
    }

    h1 {
      text-align: center;
      color: #00bcd4;
      margin-bottom: 20px;
      font-weight: 600;
    }

    label {
      display: block;
      margin-top: 10px;
      margin-bottom: 4px;
      font-weight: 500;
    }

    input[type="number"],
    input[type="time"] {
      width: 100%;
      padding: 10px;
      border: 1px solid #37474f;
      border-radius: 8px;
      font-size: 15px;
      background-color: #1e272e;
      color: #ffffff;
      outline: none;
      transition: border-color 0.2s;
    }

    input[type="number"]:focus,
    input[type="time"]:focus {
      border-color: #00bcd4;
    }

    .days-box {
      display: flex;
      flex-wrap: wrap;
      margin-top: 8px;
    }

    .days-box label {
      flex: 1 1 33%;
      display: flex;
      align-items: center;
      font-size: 15px;
      margin-bottom: 6px;
    }

    input[type="checkbox"] {
      margin-right: 8px;
      transform: scale(1.2);
    }

    button {
      width: 100%;
      padding: 12px;
      margin-top: 16px;
      border: none;
      border-radius: 10px;
      font-size: 16px;
      cursor: pointer;
      color: #fff;
      background: linear-gradient(135deg, #009688, #26c6da);
      transition: 0.2s;
    }

    button:hover {
      opacity: 0.9;
      transform: scale(1.02);
    }

    .back-btn {
      background: linear-gradient(135deg, #1565c0, #42a5f5);
    }

    #message {
      margin-top: 16px;
      white-space: pre-wrap;
      font-size: 15px;
      color: #00e676;
      text-align: center;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Twist Mode</h1>
    <form id="twistForm">
      <label>On Duration (secs)</label>
      <input type="number" name="onDuration" min="1" max="600" required />

      <label>Off Duration (secs)</label>
      <input type="number" name="offDuration" min="1" max="600" required />

      <label>On Time (HH:MM)</label>
      <input type="time" name="onTime" required />

      <label>Off Time (HH:MM)</label>
      <input type="time" name="offTime" required />

      <label>Active Days</label>
      <div class="days-box">
        <label><input type="checkbox" name="days" value="mon">Mon</label>
        <label><input type="checkbox" name="days" value="tue">Tue</label>
        <label><input type="checkbox" name="days" value="wed">Wed</label>
        <label><input type="checkbox" name="days" value="thu">Thu</label>
        <label><input type="checkbox" name="days" value="fri">Fri</label>
        <label><input type="checkbox" name="days" value="sat">Sat</label>
        <label><input type="checkbox" name="days" value="sun">Sun</label>
      </div>

      <button type="submit">Save</button>
    </form>

    <p id="message"></p>

    <a href="/"><button type="button" class="back-btn">Back</button></a>
  </div>

  <script>
    document.getElementById('twistForm').addEventListener('submit', async function(e) {
      e.preventDefault();

      const form = new FormData(this);
      const params = new URLSearchParams();

      params.append('onDuration', form.get('onDuration'));
      params.append('offDuration', form.get('offDuration'));
      params.append('onTime', form.get('onTime'));
      params.append('offTime', form.get('offTime'));

      // Collect checked days and append individually (mon=1&tue=1...)
      const selectedDays = [];
      document.querySelectorAll('input[name="days"]:checked').forEach(cb => {
        selectedDays.push(cb.value);
        params.append(cb.value, '1');
      });

      try {
        const res = await fetch('/twist_submit?' + params.toString());
        const text = await res.text();

        let msg = "Twist Settings Saved\n";
        msg += `On Duration: ${form.get('onDuration')} sec\n`;
        msg += `Off Duration: ${form.get('offDuration')} sec\n`;
        msg += `Time: ${form.get('onTime')} - ${form.get('offTime')}\n`;
        msg += `Days: ${selectedDays.join(', ') || '-'}`;
        document.getElementById('message').textContent = msg;
      } catch {
        document.getElementById('message').textContent = "Failed to send to ESP.";
      }

      this.reset();
    });
  </script>
</body>
</html>
)rawliteral";
