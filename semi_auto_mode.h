#pragma once
const char* semiAutoModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Semi Auto Mode</title>
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
      width: 250px;
      margin: 10px;
    }
    .button {
      padding: 12px 25px;
      font-size: 20px;
      background: #ffc107;
      color: black;
      border: none;
      border-radius: 8px;
    }
  </style>
</head>
<body>
  <h1>Semi Auto Control</h1>
  <form action="/semi" method="GET">
    <label>Enter Command:</label><br>
    <input type="text" name="cmd"><br>
    <input type="submit" class="button" value="Submit">
  </form>
  <a href="/"><button class="button">Back</button></a>
</body>
</html>
)rawliteral";
