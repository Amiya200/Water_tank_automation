#ifndef SETTINGS_MODE_H
#define SETTINGS_MODE_H

const char* settingsModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Device Settings</title>

<style>
:root{
  --primary:#2563eb;
  --primary2:#7c3aed;
  --danger:#dc2626;
  --success:#16a34a;
  --bg1:#eef2ff;
  --bg2:#fdf4ff;
  --border:#e5e7eb;
  --text:#0f172a;
}

*{box-sizing:border-box}

body{
  margin:0;
  font-family:Segoe UI,Roboto,Arial,sans-serif;
  background:linear-gradient(135deg,var(--bg1),var(--bg2));
  color:var(--text);
}

.wrap{max-width:1050px;margin:30px auto;padding:15px;}

.card{
  background:#ffffff;
  border-radius:20px;
  padding:32px;
  box-shadow:0 20px 40px rgba(0,0,0,.08);
}

.section{
  margin-top:28px;
  padding:24px;
  border-radius:16px;
  background:#ffffff;
  border:1px solid var(--border);
}

.section-title{
  font-size:18px;
  font-weight:700;
  margin-bottom:20px;
  color:var(--primary);
}

.setting-row{
  display:flex;
  align-items:center;
  justify-content:space-between;
  gap:20px;
  margin-bottom:16px;
  flex-wrap:wrap;
}

.setting-left{
  flex:1;
  min-width:200px;
}

.setting-right{
  display:flex;
  align-items:center;
  gap:12px;
}

input,select{
  padding:10px;
  border-radius:10px;
  border:1px solid var(--border);
  width:140px;
}

/* Toggle */
.switch {
  position: relative;
  display: inline-block;
  width: 46px;
  height: 24px;
}

.switch input { display:none; }

.slider {
  position: absolute;
  cursor: pointer;
  top: 0; left: 0; right: 0; bottom: 0;
  background-color: #ccc;
  transition: .3s;
  border-radius: 30px;
}

.slider:before {
  position: absolute;
  content: "";
  height: 18px;
  width: 18px;
  left: 3px;
  bottom: 3px;
  background-color: white;
  transition: .3s;
  border-radius: 50%;
}

input:checked + .slider {
  background:linear-gradient(135deg,var(--primary),var(--primary2));
}

input:checked + .slider:before {
  transform: translateX(22px);
}

.actions{
  display:flex;
  justify-content:space-between;
  margin-top:35px;
  gap:12px;
  flex-wrap:wrap;
}

.btn{
  padding:14px 30px;
  border-radius:14px;
  border:none;
  font-weight:700;
  cursor:pointer;
}

.btn-primary{
  background:linear-gradient(135deg,var(--primary),var(--primary2));
  color:#fff;
}

.btn-danger{
  background:#fff;
  border:2px solid var(--danger);
  color:var(--danger);
}

.status{
  margin-top:20px;
  padding:14px;
  border-radius:12px;
  border:1px solid var(--border);
  background:#fff;
}

.status.success{color:var(--success)}
.status.error{color:var(--danger)}

.disabled-input{
  opacity:0.5;
  pointer-events:none;
}

.blur-disabled{
  opacity:0.6;
  filter:blur(0.3px);
  transition:0.2s ease;
}
</style>
</head>

<body>
<div class="wrap">
<div class="card">

<h2>Device Settings</h2>

<!-- DEVICE PROTECTION -->
<div class="section">
<div class="section-title">Device Protection</div>

<div class="setting-row">
<div class="setting-left">Dry Run Gap (minutes)</div>
<div class="setting-right">
<input type="number" id="dryRunGap">
<label class="switch">
<input type="checkbox" id="dryRunGap_en" onchange="toggleInput('dryRunGap','dryRunGap_en')">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row">
<div class="setting-left">Testing Gap (minutes)</div>
<div class="setting-right">
<input type="number" id="testingGap">
<label class="switch">
<input type="checkbox" id="testingGap_en" onchange="toggleInput('testingGap','testingGap_en')">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row">
<div class="setting-left">Maximum Run Time (minutes)</div>
<div class="setting-right">
<input type="number" id="maxRun">
<label class="switch">
<input type="checkbox" id="maxRun_en" onchange="toggleInput('maxRun','maxRun_en')">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row">
<div class="setting-left">Retry Count</div>
<div class="setting-right">
<input type="number" id="retryCount">
<label class="switch">
<input type="checkbox" id="retryCount_en" onchange="toggleInput('retryCount','retryCount_en')">
<span class="slider"></span>
</label>
</div>
</div>

</div>

<!-- VOLTAGE SETTINGS -->
<div class="section">
<div class="section-title">Voltage and Load Limits</div>

<div class="setting-row">
<div class="setting-left">Low Voltage Cutoff</div>
<div class="setting-right">
<input type="number" id="lowVolt">
<label class="switch">
<input type="checkbox" id="lowVolt_en" onchange="toggleInput('lowVolt','lowVolt_en')">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row">
<div class="setting-left">High Voltage Cutoff</div>
<div class="setting-right">
<input type="number" id="highVolt">
<label class="switch">
<input type="checkbox" id="highVolt_en" onchange="toggleInput('highVolt','highVolt_en')">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row">
<div class="setting-left">Over Load Current</div>
<div class="setting-right">
<input type="number" id="overLoad">
<label class="switch">
<input type="checkbox" id="overLoad_en" onchange="toggleInput('overLoad','overLoad_en')">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row">
<div class="setting-left">Under Load Current</div>
<div class="setting-right">
<input type="number" id="underLoad">
<label class="switch">
<input type="checkbox" id="underLoad_en" onchange="toggleInput('underLoad','underLoad_en')">
<span class="slider"></span>
</label>
</div>
</div>

</div>

<!-- POWER RESTORE -->
<div class="section">
<div class="section-title">Power Restore Behaviour</div>
<div class="setting-row">
<div class="setting-left">Power Restore Mode</div>
<div class="setting-right">
<select id="powerRestore">
<option value="0">Always ON</option>
<option value="1">Stay OFF</option>
<option value="2">Restore Previous</option>
</select>
<label class="switch">
<input type="checkbox" id="powerRestore_en" onchange="toggleInput('powerRestore','powerRestore_en')">
<span class="slider"></span>
</label>
</div>
</div>
</div>

<!-- BUZZER SETTINGS -->
<div class="section">
<div class="section-title">Buzzer Control</div>

<div class="setting-row">
<div class="setting-left">Main Buzzer</div>
<div class="setting-right">
<label class="switch">
<input type="checkbox" id="buzzerEnable" onchange="updateBuzzerOptions()">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row" id="rowTankFull">
<div class="setting-left">Water Tank Full Alert</div>
<div class="setting-right">
<label class="switch">
<input type="checkbox" id="buzzerTankFull">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row" id="rowTankEmpty">
<div class="setting-left">Water Tank Empty Alert</div>
<div class="setting-right">
<label class="switch">
<input type="checkbox" id="buzzerTankEmpty">
<span class="slider"></span>
</label>
</div>
</div>

<div class="setting-row" id="rowMotorRunning">
<div class="setting-left">Motor Running Alert</div>
<div class="setting-right">
<label class="switch">
<input type="checkbox" id="buzzerMotorRunning">
<span class="slider"></span>
</label>
</div>
</div>

</div>

<div class="actions">
<button class="btn btn-danger" onclick="factoryReset()">Factory Reset</button>
<button id="applyBtn" class="btn btn-primary" onclick="apply()">Apply Settings</button>
</div>

<div id="status" class="status">Loading...</div>

</div>
</div>

<script>

/* FIX: was '/settings/get' — correct endpoint is '/get_settings' */
window.onload = function(){

fetch('/get_settings')
.then(res=>res.json())
.then(data=>{

let ids=[
"dryRunGap","dryRunGap_en",
"testingGap","testingGap_en",
"maxRun","maxRun_en",
"retryCount","retryCount_en",
"lowVolt","lowVolt_en",
"highVolt","highVolt_en",
"overLoad","overLoad_en",
"underLoad","underLoad_en",
"powerRestore","powerRestore_en",
"buzzerEnable","buzzerTankFull",
"buzzerTankEmpty","buzzerMotorRunning"
];

ids.forEach(id=>{
if(data[id]!==undefined){
let el=document.getElementById(id);
if(el.type==="checkbox")
el.checked=data[id]==1;
else
el.value=data[id];
}
});

ids.forEach(id=>{
if(id.endsWith("_en"))
toggleInput(id.replace("_en",""),id);
});

updateBuzzerOptions();

document.getElementById("status").innerText="Ready";

})
.catch(()=>{
document.getElementById("status").innerText="Failed to load settings";
});
};

function toggleInput(inputId,toggleId){
let input=document.getElementById(inputId);
let toggle=document.getElementById(toggleId);
if(!toggle.checked)
input.classList.add("disabled-input");
else
input.classList.remove("disabled-input");
}

function updateBuzzerOptions(){

let main=document.getElementById("buzzerEnable").checked;

let items=[
{checkbox:"buzzerTankFull",row:"rowTankFull"},
{checkbox:"buzzerTankEmpty",row:"rowTankEmpty"},
{checkbox:"buzzerMotorRunning",row:"rowMotorRunning"}
];

items.forEach(item=>{
let el=document.getElementById(item.checkbox);
let row=document.getElementById(item.row);

if(!main){
el.checked=false;
el.disabled=true;
row.classList.add("blur-disabled");
}else{
el.disabled=false;
row.classList.remove("blur-disabled");
}
});
}

/* UART KEY MAP */

const keyMap = {

dryRunGap:"D",
dryRunGap_en:"DE",

testingGap:"T",
testingGap_en:"TE",

maxRun:"M",
maxRun_en:"ME",

retryCount:"RC",
retryCount_en:"RCE",

lowVolt:"LV",
lowVolt_en:"LVE",

highVolt:"HV",
highVolt_en:"HVE",

overLoad:"OL",
overLoad_en:"OLE",

underLoad:"UL",
underLoad_en:"ULE",

powerRestore:"PR",
powerRestore_en:"PRE",

buzzerEnable:"BZ",
buzzerTankFull:"BF",
buzzerTankEmpty:"BE",
buzzerMotorRunning:"BR"

};

function apply(){

const btn=document.getElementById("applyBtn");
const status=document.getElementById("status");

let ids=[
"dryRunGap","dryRunGap_en",
"testingGap","testingGap_en",
"maxRun","maxRun_en",
"retryCount","retryCount_en",
"lowVolt","lowVolt_en",
"highVolt","highVolt_en",
"overLoad","overLoad_en",
"underLoad","underLoad_en",
"powerRestore","powerRestore_en",
"buzzerEnable","buzzerTankFull",
"buzzerTankEmpty","buzzerMotorRunning"
];

let p=[];

ids.forEach(id=>{

let el=document.getElementById(id);

let val=(el.type==="checkbox")?(el.checked?1:0):el.value;

let key = keyMap[id] || id;

p.push(key+"="+val);

});

btn.disabled=true;

status.innerText="Saving settings...";

fetch("/settings/set?data="+encodeURIComponent(p.join(";")))
.then(r=>r.text())
.then(t=>{
status.className="status success";
status.innerText=t;
})
.catch(()=>{
status.className="status error";
status.innerText="Failed to save settings";
})
.finally(()=>btn.disabled=false);

}

function factoryReset(){
if(confirm("Reset device to factory default?")){
fetch("/factory_reset")
.then(()=>alert("Device reset completed"));
}
}

</script>

</body>
</html>
)rawliteral";

#endif