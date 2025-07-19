#pragma once
const char* searchModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Search Mode</title>
  <style>
    body {
      background-color: #000;
      color: white;
      font-family: sans-serif;
      padding: 40px;
      text-align: center;
    }
    input[type="text"] {
      font-size: 20px;
      padding: 10px;
      width: 300px;
      margin: 20px 0;
    }
    .button {
      padding: 15px 30px;
      background: #6610f2;
      color: white;
      border: none;
      border-radius: 8px;
      font-size: 20px;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <h1>Search Mode</h1>
  <form action="/search_input" method="GET">
    <label for="query">Enter Search Text:</label><br>
    <input type="text" id="query" name="query"><br>
    <input type="submit" class="button" value="Submit">
  </form>
  <a href="/"><button class="button">Back</button></a>
</body>
</html>
)rawliteral";
