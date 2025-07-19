#pragma once
const char* countdownModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Countdown Mode</title>
  <style>
    body {
      background-color: #000;
      color: white;
      font-family: sans-serif;
      text-align: center;
      padding: 40px;
    }
    input {
      padding: 10px;
      font-size: 20px;
      width: 200px;
    }
    .button {
      padding: 12px 25px;
      font-size: 20px;
      background: #fd7e14;
      color: white;
      border: none;
      border-radius: 8px;
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <h1>Countdown Mode</h1>
  <form action="/start_countdown" method="GET">
    <label>Enter Duration (minutes):</label><br><br>
    <input type="number" name="duration" min="1" max="180"><br>
    <input type="submit" class="button" value="Start">
  </form>
  <br>
  <a href="/"><button class="button">Back</button></a>
</body>
</html>
)rawliteral";
