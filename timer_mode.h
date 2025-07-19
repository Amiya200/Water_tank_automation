#ifndef TIMER_MODE_H
#define TIMER_MODE_H

const char* timerModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Timer Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background-color: #eaecee ;
      color: white;
      text-align: center;
      padding: 20px;
    }
    .form-container {
      margin: 30px auto;
      max-width: 400px;
      background: #111;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px #0dcaf0;
    }
    h1 {
      color: #0dcaf0;
      margin-bottom: 20px;
    }
    label {
      font-size: 18px;
    }
    input[type="time"] {
      width: 45%;
      padding: 10px;
      margin: 10px;
      font-size: 16px;
      border-radius: 5px;
      border: none;
      background: #222;
      color: white;
    }
    button {
      padding: 10px 20px;
      background-color: #0dcaf0;
      color: black;
      border: none;
      border-radius: 5px;
      font-weight: bold;
      font-size: 16px;
      cursor: pointer;
      margin-top: 10px;
    }
    .result {
      margin-top: 20px;
      font-size: 18px;
      font-weight: bold;
    }
    .success { color: #28a745; }
    .error { color: #dc3545; }
    .loading {
      font-size: 14px;
      color: #888;
      margin-top: 5px;
    }
  </style>
</head>
<body>
  <h1>Set Timer Mode</h1>

  <div class="form-container">
    <form id="timerForm">
      <label>ON Time:</label><br>
      <input type="time" id="onTime" required><br>
      <label>OFF Time:</label><br>
      <input type="time" id="offTime" required><br>
      <button type="submit">Set Timer</button>
    </form>
    <div class="loading" id="loading" style="display: none;">Sending request...</div>
    <div id="result" class="result"></div>
  </div>

  <script>
    document.getElementById('timerForm').addEventListener('submit', function(e) {
      e.preventDefault();
      const onTime = document.getElementById('onTime').value;
      const offTime = document.getElementById('offTime').value;
      const loading = document.getElementById('loading');
      const result = document.getElementById('result');

      console.log("Timer form submitted with:", onTime, offTime);

      if (!onTime || !offTime) {
        result.className = 'result error';
        result.innerHTML = "❌ Please fill both ON and OFF times.";
        return;
      }

      loading.style.display = 'block';
      result.innerHTML = '';

      fetch('/timer/set?on=' + onTime + '&off=' + offTime)
        .then(function(response) {
          loading.style.display = 'none';
          if (response.ok) {
            result.className = 'result success';
            result.innerHTML =
              "✅ <b>Timer Set Successfully</b><br>ON at <b>" + onTime + "</b><br>OFF at <b>" + offTime + "</b>";
          } else {
            result.className = 'result error';
            result.innerHTML = "❌ Failed to set timer.";
          }
        })
        .catch(function() {
          loading.style.display = 'none';
          result.className = 'result error';
          result.innerHTML = "❌ Failed to connect to server.";
        });
    });
  </script>
</body>
</html>
)rawliteral";

#endif // TIMER_MODE_H