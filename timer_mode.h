#ifndef TIMER_MODE_H
#define TIMER_MODE_H

#include <Arduino.h>

#if defined(ESP8266)
  #include <pgmspace.h>
  // store in PROGMEM on ESP8266
  const char timerModeHtml[] PROGMEM = R"rawliteral(
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
      margin-bottom: 5px;
      color: #ccc;
    }
    input[type="checkbox"] {
      transform: scale(1.2);
      margin-right: 6px;
    }
    .days-container label {
      margin-right: 10px;
      display: inline-block;
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
<script>
function slotHTML(n){
  return `
  <div class="time-slot">
    <h3>Time Slot ${n}</h3>

    <label><input type="checkbox" id="enable${n}"> Enable Slot ${n}</label>

    <div style="margin-top:10px;text-align:left;color:#ccc;font-size:14px;">
      <b>Select Days:</b><br><br>
      <div class="days-container">
        <label><input type="checkbox" id="d${n}_mon"> Mon</label>
        <label><input type="checkbox" id="d${n}_tue"> Tue</label>
        <label><input type="checkbox" id="d${n}_wed"> Wed</label>
        <label><input type="checkbox" id="d${n}_thu"> Thu</label>
        <label><input type="checkbox" id="d${n}_fri"> Fri</label>
        <label><input type="checkbox" id="d${n}_sat"> Sat</label>
        <label><input type="checkbox" id="d${n}_sun"> Sun</label>
      </div>
    </div>

    <label>ON Time:</label>
    <input type="time" id="onTime${n}"><br>

    <label>OFF Time:</label>
    <input type="time" id="offTime${n}">

    <div class="optional" style="font-size:12px;color:#888;font-style:italic;margin-top:5px;">
      (Required when slot enabled)
    </div>
  </div>`;
}

document.write(slotHTML(1));
document.write(slotHTML(2));
document.write(slotHTML(3));
document.write(slotHTML(4));
document.write(slotHTML(5));
</script>

<button type="submit">Set Timer</button>
</form>

<div class="loading" id="loading" style="display:none;">Sending timer configuration...</div>
<div id="result" class="result"></div>

<button class="back-bottom" type="button" onclick="window.location.href='/'">Back to Home</button>

</div>

<script>

document.getElementById('timerForm').addEventListener('submit', function(e){
  e.preventDefault();

  const loading = document.getElementById('loading');
  const result = document.getElementById('result');

  let queryParams = [];
  let anyEnabled = false;
  let badSlots = [];

  function getDays(n){
    let arr = [];
    if(document.getElementById(`d${n}_mon`).checked) arr.push("mon");
    if(document.getElementById(`d${n}_tue`).checked) arr.push("tue");
    if(document.getElementById(`d${n}_wed`).checked) arr.push("wed");
    if(document.getElementById(`d${n}_thu`).checked) arr.push("thu");
    if(document.getElementById(`d${n}_fri`).checked) arr.push("fri");
    if(document.getElementById(`d${n}_sat`).checked) arr.push("sat");
    if(document.getElementById(`d${n}_sun`).checked) arr.push("sun");
    return arr;
  }

  for(let n=1;n<=5;n++){
    let enable = document.getElementById(`enable${n}`).checked;
    let on = document.getElementById(`onTime${n}`).value;
    let off = document.getElementById(`offTime${n}`).value;
    let days = getDays(n);

    if(enable){
      anyEnabled = true;

      if(!on || !off || days.length === 0){
        badSlots.push(n);
      } else {
        queryParams.push(`slot${n}=1`);
        queryParams.push(`days${n}=` + encodeURIComponent(days.join(",")));
        queryParams.push(`on${n}=` + encodeURIComponent(on));
        queryParams.push(`off${n}=` + encodeURIComponent(off));
      }
    }
  }

  if(!anyEnabled){
    result.className = "result error";
    result.innerHTML = "Enable at least 1 time slot.";
    return;
  }

  if(badSlots.length > 0){
    result.className = "result error";
    result.innerHTML = "Incomplete slot(s): " + badSlots.join(', ') +
      ". Please fill Days + ON + OFF when enabled.";
    return;
  }

  loading.style.display = "block";
  result.innerHTML = "";

  fetch("/timer/set?" + queryParams.join("&"))
    .then(res=>{
      loading.style.display="none";
      if(res.ok) return res.text();
      result.className="result error";
      result.innerHTML="Server error: "+res.status;
      throw new Error("HTTP "+res.status);
    })
    .then(text=>{
      result.className="result success";
      result.innerHTML="Saved Successfully:<br><br><pre>"+text+"</pre>";
    })
    .catch(err=>{
      loading.style.display="none";
      result.className="result error";
      result.innerHTML="Connection Error: "+err.message;
    });

});

</script>

</body>
</html>
)rawliteral";

#else
  // Non-ESP8266: keep as normal const char*
  const char* timerModeHtml = R"rawliteral(
  // (same HTML as above) ...
)rawliteral";
#endif

#endif // TIMER_MODE_H
