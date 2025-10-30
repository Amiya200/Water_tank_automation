#pragma once
const char* searchModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Search Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      background-color: #0a0a0a;
      color: #ffffff;
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
    }

    .container {
      max-width: 420px;
      margin: 50px auto;
      background-color: #1e1e1e;
      padding: 30px 25px;
      border-radius: 12px;
      box-shadow: 0 0 15px rgba(102, 16, 242, 0.3);
    }

    h1 {
      font-size: 24px;
      color: #a970ff;
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
      color: #ffffff;
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
      display: flex;
      align-items: center;
      font-size: 15px;
      margin: 6px 0;
    }

    input[type="checkbox"] {
      margin-right: 8px;
      transform: scale(1.2);
    }

    .button {
      width: 100%;
      padding: 12px;
      font-size: 18px;
      background-color: #6610f2;
      color: white;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      transition: 0.3s;
      margin-bottom: 12px;
    }

    .button:hover {
      background-color: #8148fa;
    }

    .back-button {
      background-color: #0dcaf0;
    }

    #message {
      margin-top: 20px;
      text-align: center;
      font-size: 16px;
      color: #00ff88;
      white-space: pre-wrap;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Search Mode Settings</h1>
    <form id="searchForm">
      <label for="gap">Testing Gap (MM:SS)</label>
      <input type="text" id="gap" name="gap" placeholder="e.g., 00:30" required>

      <label for="dryrun">Dry Run Time (MM:SS)</label>
      <input type="text" id="dryrun" name="dryrun" placeholder="e.g., 00:03" required>

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
    document.getElementById('searchForm').addEventListener('submit', async function(e) {
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

      const response = await fetch('/search_submit?' + params.toString());
      const result = await response.text();

      // Build summary display
      let summary = `✅ Search Mode Settings Saved\n`;
      summary += `Testing Gap: ${dataSummary.gap || '-'}\n`;
      summary += `Dry Run Time: ${dataSummary.dryrun || '-'}\n`;
      summary += `ON Time: ${dataSummary.onTime || '-'}\n`;
      summary += `OFF Time: ${dataSummary.offTime || '-'}\n`;
      summary += `Active Days: ${(dataSummary.days || []).join(', ') || '-'}`;

      document.getElementById('message').textContent = summary;

      this.reset();
    });
  </script>
</body>
</html>
)rawliteral";
