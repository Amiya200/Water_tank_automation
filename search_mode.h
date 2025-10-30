#pragma once
const char* searchModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Search Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body { background:#f4f6f7; font-family: 'Segoe UI', sans-serif; margin:0; }
    .container {
      max-width:420px; margin:20px auto; background:#fff; padding:20px;
      border-radius:14px; box-shadow:0 4px 16px rgba(0,0,0,.12);
    }
    h1 { text-align:center; color:#2c3e50; }
    label { display:block; margin-top:10px; }
    input, select {
      width:100%; padding:8px; border:1px solid #d5d8dc; border-radius:8px;
    }
    .days-box label { display:inline-block; margin-right:8px; }
    button {
      width:100%; padding:12px; margin-top:16px; background:#28b463; color:#fff;
      border:none; border-radius:10px; font-size:15px; cursor:pointer;
    }
    #message { margin-top:16px; white-space:pre-wrap; font-size:14px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Search Mode</h1>
    <form id="searchForm">
      <label>Testing Gap (mins)</label>
      <input type="number" name="gap" min="1" max="300" required />

      <label>Dry Run Time (secs)</label>
      <input type="number" name="dryrun" min="1" max="300" required />

      <label>Days</label>
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
    <a href="/"><button type="button">Back</button></a>
  </div>

  <script>
    document.getElementById('searchForm').addEventListener('submit', async function(e) {
      e.preventDefault();

      const form = new FormData(this);
      const params = new URLSearchParams();
      const summary = { days: [] };

      for (const [k, v] of form.entries()) {
        if (k === 'days') {
          summary.days.push(v);
          params.append(k, v);
        } else {
          summary[k] = v;
          params.append(k, v);
        }
      }

      try {
        const response = await fetch('/search_submit?' + params.toString());
        const text = await response.text();
        let msg = "Settings saved\n";
        msg += `Testing Gap: ${summary.gap || '-'}\n`;
        msg += `Dry Run Time: ${summary.dryrun || '-'}\n`;
        msg += `Days: ${summary.days.join(', ') || '-'}`;
        document.getElementById('message').textContent = msg;
      } catch (err) {
        document.getElementById('message').textContent = "Failed to send to ESP.";
      }
    });
  </script>
</body>
</html>
)rawliteral";
