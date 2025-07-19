#pragma once
const char* manualModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Manual Mode</title>
  <style>
    body {
      background-color: #000;
      color: white;
      font-family: sans-serif;
      text-align: center;
      padding: 40px;
    }
    .button {
      padding: 15px 30px;
      margin: 15px;
      font-size: 20px;
      background: #0d6efd;
      border: none;
      color: white;
      border-radius: 8px;
    }
  </style>
</head>
<body>
  <h1>Manual Control</h1>
  <button class="button" onclick="fetch('/manual/on')">Turn ON</button>
  <button class="button" onclick="fetch('/manual/off')">Turn OFF</button>
  <br><br>
  <a href="/"><button class="button">Back</button></a>
</body>
</html>
)rawliteral";
