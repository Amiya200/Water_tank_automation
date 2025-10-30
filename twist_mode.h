#pragma once
const char* twistModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Twist Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body { background:#fdfefe; font-family:'Segoe UI',sans-serif; margin:0; }
    .container {
      max-width:420px; margin:20px auto; background:#fff; padding:20px;
      border-radius:14px; box-shadow:0 4px 16px rgba(0,0,0,.12);
    }
    h1 { text-align:center; color:#2c3e50; }
    label { display:block; margin-top:10px; }
    input {
      width:100%; padding:8px; border:1px solid #d5d8dc; border-radius:8px;
    }
    .days-box label { display:inline-block; margin-right:8px; }
    button {
      width:100%; padding:12px; margin-top:16px; background:#9b59b6; color:#fff;
      border:none; border-radius:10px; font-size:15px; cursor:pointer;
    }
    #message { margin-top:16px; white-space:pre-wrap; font-size:14px; }
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
    document.getElementById('twistForm').addEventListener('submit', async function(e) {
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
        const res = await fetch('/twist_submit?' + params.toString());
        const text = await res.text();

        let msg = "Twist Settings Saved\n";
        msg += `On Duration: ${summary.onDuration || '-'} sec\n`;
        msg += `Off Duration: ${summary.offDuration || '-'} sec\n`;
        msg += `Time: ${summary.onTime || '-'} - ${summary.offTime || '-'}\n`;
        msg += `Days: ${summary.days.join(', ') || '-'}`;
        document.getElementById('message').textContent = msg;
      } catch (err) {
        document.getElementById('message').textContent = "Failed to send to ESP.";
      }

      this.reset();
    });
  </script>
</body>
</html>
)rawliteral";
