const char* timerModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Timer Mode</title>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background-color: #000;
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
    }
    input[type="time"] {
      width: 45%;
      padding: 10px;
      margin: 10px;
      font-size: 16px;
      border-radius: 5px;
      border: none;
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
    }
    .result {
      margin-top: 20px;
      font-size: 18px;
      font-weight: bold;
      color: #28a745;
    }
  </style>
</head>
<body>
  <h1>Timer Mode</h1>
  <div class="form-container">
    <form id="timerForm">
      <label>Set ON Time:</label><br>
      <input type="time" id="onTime" required><br>
      <label>Set OFF Time:</label><br>
      <input type="time" id="offTime" required><br>
      <button type="submit">Set Timer</button>
    </form>
    <div id="result" class="result"></div>
  </div>

  <script>
    document.getElementById('timerForm').addEventListener('submit', function(e) {
      e.preventDefault();
      var onTime = document.getElementById('onTime').value;
      var offTime = document.getElementById('offTime').value;

      fetch('/timer/set?on=' + onTime + '&off=' + offTime)
        .then(function(response) {
          if (response.ok) {
            document.getElementById('result').innerHTML =
              "✅ Timer Set: ON at <b>" + onTime + "</b>, OFF at <b>" + offTime + "</b>";
          } else {
            document.getElementById('result').innerHTML =
              "❌ Failed to set timer.";
          }
        })
        .catch(function() {
          document.getElementById('result').innerHTML = "❌ Failed to connect.";
        });
    });
  </script>
</body>
</html>
)rawliteral";
