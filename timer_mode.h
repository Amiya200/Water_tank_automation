#pragma once
const char* timerModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Timer Mode</title>
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
      margin: 10px;
    }
    .button {
      padding: 12px 25px;
      font-size: 20px;
      background: #0dcaf0;
      color: white;
      border: none;
      border-radius: 8px;
    }
  </style>
</head>
<body>
  <h1>Timer Mode</h1>
  <form action="/set_timer" method="GET">
    <label>Start Time:</label><br>
    <input type="time" name="start"><br>
    <label>Stop Time:</label><br>
    <input type="time" name="stop"><br><br>
    <input type="submit" class="button" value="Set Timer">
  </form>
  <a href="/"><button class="button">Back</button></a>
</body>
</html>
)rawliteral";
