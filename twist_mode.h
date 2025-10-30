#pragma once
const char* twistModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Twist Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background-color: #0a0a0a;
      color: white;
      font-family: 'Segoe UI', sans-serif;
      padding: 20px;
      margin: 0;
    }

    .container {
      max-width: 450px;
      margin: 50px auto;
      background-color: #1c1c1c;
      padding: 30px 25px;
      border-radius: 12px;
      box-shadow: 0 0 15px rgba(32, 201, 151, 0.3);
    }

    h1 {
      font-size: 24px;
      color: #20c997;
      margin-bottom: 25px;
      text-align: center;
    }

    label {
      display: block;
      margin-bottom: 8px;
      font-size: 16px;
      text-align: left;
    }

    input[type="text"],
    input[type="time"] {
      width: 100%;
      padding: 10px;
      font-size: 16px;
      border: none;
      border-radius: 6px;
      background-color: #2d2d2d;
      color: white;
      margin-bottom: 20px;
    }

    .day-checkboxes {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      margin-bottom: 20px;
    }

    .day-checkboxes label {
      flex: 1 1 30%;
      font-size: 15px;
      margin: 6px 0;
    }

    input[type="checkbox"] {
      margin-right: 6px;
      transform: scale(1.1);
    }

    .button {
      width: 100%;
      padding: 12px;
      font-size: 18px;
      background-color: #20c997;
      border: none;
      border-radius: 8px;
      color: white;
      cursor: pointer;
      transition: 0.3s;
      margin-top: 10px;
    }

    .button:hover {
      background-color: #38e3b6;
    }

    .back-button {
      background-color: #0dcaf0;
      margin-top: 15px;
    }

    #message {
      margin-top: 20px;
      text-align: center;
      font-size: 15px;
      color: #0f0;
      white-space: pre-wrap;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Twist Mode Settings</h1>
    <form id="twistForm">
      <label for="onDuration">On Duration (MM:SS)</label>
      <input type="text" id="onDuration" name="onDuration" placeholder="e.g., 01:30" required>

      <label for="offDuration">Off Duration (MM:SS)</label>
      <input type="text" id="offDuration" name="offDuration" placeholder="e.g., 00:30" required>

      <label for="onTime">Set ON Time</label>
      <input type="time" id="onTime" name="onTime" required>

      <label for="offTime">Set OFF Time</label>
      <input type="time" id="offTime" name="offTime" required>

      <label>Active Days</label>
      <div class="day-checkboxes">
        <label><input type="checkbox" name="days" value="sun">Sun</label>
        <label><input type="checkbox" name="days" value="mon">Mon</label>
        <label><input type="checkbox" name="days" value="tue">Tue</label>
        <label><input type="checkbox" name="days" value="wed">Wed</label>
        <label><input type="checkbox" name="days" value="thu">Thu</label>
        <label><input type="checkbox" name="days" value="fri">Fri</label>
        <label><input type="checkbox" name="days" value="sat">Sat</label>
      </div>

      <button type="submit" class="button">Save Settings</button>
    </form>

    <p id="message"></p>

    <a href="/"><button class="button back-button">Back</button></a>
  </div>

  <script>
    document.getElementById('twistForm').addEventListener('submit', async function(e) {
      e.preventDefault();

      const form = new FormData(this);
      const params = new URLSearchParams();
      const dataSummary = {};

      for (const pair of form.entries()) {
        const [key, value] = pair;
        if (key === 'days') {
          if (!dataSummary.days) dataSummary.days = [];
          dataSummary.days.push(value);
        } else {
          dataSummary[key] = value;
        }
        params.append(key, value);
      }

      const res = await fetch('/twist_submit?' + params.toString());
      const result = await res.text();

      // Show what was submitted
      let summary = ` Twist Settings Saved\n`;
      summary += `• On Duration: ${dataSummary.onDuration || '-'}\n`;
      summary += `• Off Duration: ${dataSummary.offDuration || '-'}\n`;
      summary += `• Time: ${dataSummary.onTime || '-'} - ${dataSummary.offTime || '-'}\n`;
      summary += `• Days: ${(dataSummary.days || []).join(', ') || '-'}`;
      document.getElementById('message').textContent = summary;

      this.reset(); // clear the form
    });
  </script>
</body>
</html>
)rawliteral";
