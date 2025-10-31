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
      background-color: #eaecee;
      color: white;
      text-align: center;
      padding: 20px;
      margin: 0;
    }
    h1 {
      color: #0dcaf0;
      margin-bottom: 25px;
      font-size: 28px;
    }
    .form-container {
      margin: 0 auto 30px;
      max-width: 500px;
      background: #111;
      padding: 25px;
      border-radius: 12px;
      box-shadow: 0 0 15px #0dcaf0;
    }
    .time-slot {
      margin: 15px 0;
      padding: 15px;
      background: #1a1a1a;
      border-radius: 8px;
      border-left: 4px solid #0dcaf0;
    }
    .time-slot h3 {
      color: #0dcaf0;
      margin: 0 0 12px 0;
      font-size: 18px;
    }
    label {
      font-size: 16px;
      display: block;
      margin-bottom: 5px;
      color: #ccc;
    }
    input[type="time"] {
      width: 45%;
      padding: 12px;
      margin: 8px;
      font-size: 16px;
      border-radius: 6px;
      border: 2px solid #333;
      background: #222;
      color: white;
      transition: border-color 0.3s ease;
    }
    input[type="time"]:focus {
      border-color: #0dcaf0;
      outline: none;
    }
    button {
      padding: 12px 25px;
      background: linear-gradient(135deg, #0dcaf0, #0099cc);
      color: white;
      border: none;
      border-radius: 8px;
      font-weight: bold;
      font-size: 16px;
      cursor: pointer;
      margin-top: 20px;
      transition: transform 0.2s ease, box-shadow 0.2s ease;
      box-shadow: 0 4px 8px rgba(13, 202, 240, 0.3);
    }
    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 6px 12px rgba(13, 202, 240, 0.4);
    }
    .result {
      margin-top: 20px;
      font-size: 16px;
      font-weight: bold;
      padding: 15px;
      border-radius: 8px;
      background: #1a1a1a;
    }
    .success {
      color: #28a745;
      border: 2px solid #28a745;
    }
    .error {
      color: #dc3545;
      border: 2px solid #dc3545;
    }
    .loading {
      font-size: 14px;
      color: #0dcaf0;
      margin-top: 10px;
      font-style: italic;
    }
    .back-bottom {
      background: #222;
      margin-top: 18px;
      border: 1px solid #0dcaf0;
    }
  </style>
</head>
<body>
  <h1>Set Timer Mode</h1>

  <div class="form-container">
    <form id="timerForm">
      <div class="time-slot">
        <h3>Time Slot 1</h3>
        <label>ON Time:</label>
        <input type="time" id="onTime1"><br>
        <label>OFF Time:</label>
        <input type="time" id="offTime1">
        <div class="optional" style="font-size:12px;color:#888;font-style:italic;margin-top:5px;">(Optional - fill both or none)</div>
      </div>

      <div class="time-slot">
        <h3>Time Slot 2</h3>
        <label>ON Time:</label>
        <input type="time" id="onTime2"><br>
        <label>OFF Time:</label>
        <input type="time" id="offTime2">
        <div class="optional" style="font-size:12px;color:#888;font-style:italic;margin-top:5px;">(Optional - fill both or none)</div>
      </div>

      <div class="time-slot">
        <h3>Time Slot 3</h3>
        <label>ON Time:</label>
        <input type="time" id="onTime3"><br>
        <label>OFF Time:</label>
        <input type="time" id="offTime3">
        <div class="optional" style="font-size:12px;color:#888;font-style:italic;margin-top:5px;">(Optional - fill both or none)</div>
      </div>

      <div class="time-slot">
        <h3>Time Slot 4</h3>
        <label>ON Time:</label>
        <input type="time" id="onTime4"><br>
        <label>OFF Time:</label>
        <input type="time" id="offTime4">
        <div class="optional" style="font-size:12px;color:#888;font-style:italic;margin-top:5px;">(Optional - fill both or none)</div>
      </div>

      <div class="time-slot">
        <h3>Time Slot 5</h3>
        <label>ON Time:</label>
        <input type="time" id="onTime5"><br>
        <label>OFF Time:</label>
        <input type="time" id="offTime5">
        <div class="optional" style="font-size:12px;color:#888;font-style:italic;margin-top:5px;">(Optional - fill both or none)</div>
      </div>

      <button type="submit">Set Timer</button>
    </form>
    <div class="loading" id="loading" style="display: none;">Sending timer configuration...</div>
    <div id="result" class="result"></div>

    <!-- bottom back only -->
    <button class="back-bottom" type="button" onclick="window.location.href='/'">Back to Home</button>
  </div>

  <script>
    document.getElementById('timerForm').addEventListener('submit', function(e) {
      e.preventDefault();

      const onTime1 = document.getElementById('onTime1').value;
      const offTime1 = document.getElementById('offTime1').value;
      const onTime2 = document.getElementById('onTime2').value;
      const offTime2 = document.getElementById('offTime2').value;
      const onTime3 = document.getElementById('onTime3').value;
      const offTime3 = document.getElementById('offTime3').value;
      const onTime4 = document.getElementById('onTime4').value;
      const offTime4 = document.getElementById('offTime4').value;
      const onTime5 = document.getElementById('onTime5').value;
      const offTime5 = document.getElementById('offTime5').value;

      const loading = document.getElementById('loading');
      const result = document.getElementById('result');

      const has1 = onTime1 && offTime1;
      const has2 = onTime2 && offTime2;
      const has3 = onTime3 && offTime3;
      const has4 = onTime4 && offTime4;
      const has5 = onTime5 && offTime5;

      if (!has1 && !has2 && !has3 && !has4 && !has5) {
        result.className = 'result error';
        result.innerHTML = "Please fill at least one complete time slot (both ON and OFF times)";
        return;
      }

      const bad = [];
      if ((onTime1 && !offTime1) || (!onTime1 && offTime1)) bad.push(1);
      if ((onTime2 && !offTime2) || (!onTime2 && offTime2)) bad.push(2);
      if ((onTime3 && !offTime3) || (!onTime3 && offTime3)) bad.push(3);
      if ((onTime4 && !offTime4) || (!onTime4 && offTime4)) bad.push(4);
      if ((onTime5 && !offTime5) || (!onTime5 && offTime5)) bad.push(5);

      if (bad.length > 0) {
        result.className = 'result error';
        result.innerHTML = "Incomplete time slot(s): " + bad.join(', ') +
                           ". Please fill both ON and OFF or leave both empty.";
        return;
      }

      let queryParams = [];
      if (has1) {
        queryParams.push('on1=' + encodeURIComponent(onTime1));
        queryParams.push('off1=' + encodeURIComponent(offTime1));
      }
      if (has2) {
        queryParams.push('on2=' + encodeURIComponent(onTime2));
        queryParams.push('off2=' + encodeURIComponent(offTime2));
      }
      if (has3) {
        queryParams.push('on3=' + encodeURIComponent(onTime3));
        queryParams.push('off3=' + encodeURIComponent(offTime3));
      }
      if (has4) {
        queryParams.push('on4=' + encodeURIComponent(onTime4));
        queryParams.push('off4=' + encodeURIComponent(offTime4));
      }
      if (has5) {
        queryParams.push('on5=' + encodeURIComponent(onTime5));
        queryParams.push('off5=' + encodeURIComponent(offTime5));
      }

      loading.style.display = 'block';
      result.innerHTML = '';

      fetch('/timer/set?' + queryParams.join('&'))
        .then(function(response) {
          loading.style.display = 'none';
          if (response.ok) {
            return response.text();
          } else {
            result.className = 'result error';
            result.innerHTML = "Failed to save timer configuration. Server returned: " + response.status;
            throw new Error('Server returned ' + response.status);
          }
        })
        .then(function(text) {
          result.className = 'result success';
          result.innerHTML = "Timer configuration saved successfully!<br><br><pre style='text-align:left;white-space:pre-wrap;'>" + text + "</pre>";
        })
        .catch(function(err) {
          loading.style.display = 'none';
          result.className = 'result error';
          result.innerHTML = "Connection error: " + err.message;
        });
    });
  </script>
</body>
</html>
)rawliteral";

#endif // TIMER_MODE_H
