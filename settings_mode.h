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
  --card:#ffffff;
  --border:#e5e7eb;
  --text:#0f172a;
  --muted:#64748b;
}
*{box-sizing:border-box}

body{
  margin:0;
  font-family:Inter,Segoe UI,Roboto,Arial,sans-serif;
  background:linear-gradient(135deg,var(--bg1),var(--bg2));
  color:var(--text);
}

.wrap{max-width:920px;margin:32px auto;padding:16px;}

.card{
  background:linear-gradient(180deg,#ffffff,#f9fafb);
  border-radius:22px;
  padding:26px;
  box-shadow:0 20px 40px rgba(0,0,0,.08);
}

.section{
  margin-top:30px;
  padding:22px;
  border-radius:18px;
  background:#ffffff;
  border:1px solid var(--border);
}

.section-title{
  font-size:16px;
  font-weight:800;
  margin-bottom:16px;
}

.row{display:flex;gap:18px;flex-wrap:wrap;}
.col{flex:1;min-width:180px;}

label{
  font-size:13px;
  font-weight:700;
  display:block;
  margin-bottom:6px;
}

input,select{
  width:100%;
  padding:10px;
  border-radius:12px;
  border:1px solid var(--border);
  margin-bottom:8px;
}

.actions{
  display:flex;
  justify-content:space-between;
  margin-top:30px;
  gap:12px;
  flex-wrap:wrap;
}

.btn{
  padding:14px 26px;
  border-radius:16px;
  border:none;
  font-weight:800;
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
  padding:16px;
  border-radius:16px;
  background:#fff;
  border:1px solid var(--border);
}

.status.success{color:var(--success)}
.status.error{color:var(--danger)}
</style>
</head>

<body>
<div class="wrap">
<div class="card">

<h2>Device Settings</h2>

<!-- ================= DEVICE PROTECTION ================= -->
<div class="section">
<div class="section-title">Device Protection</div>

<div class="row">
<div class="col">
<label>Dry Run Gap (minutes)</label>
<input id="dryRunGap" type="number">
<select id="dryRunGap_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>

<div class="col">
<label>Testing Gap (minutes)</label>
<input id="testingGap" type="number">
<select id="testingGap_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>

<div class="col">
<label>Maximum Run Time (minutes)</label>
<input id="maxRun" type="number">
<select id="maxRun_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>
</div>
</div>

<!-- ================= VOLTAGE AND LOAD ================= -->
<div class="section">
<div class="section-title">Voltage and Load Limits</div>

<div class="row">
<div class="col">
<label>Low Voltage Cutoff</label>
<input id="lowVolt" type="number">
<select id="lowVolt_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>

<div class="col">
<label>High Voltage Cutoff</label>
<input id="highVolt" type="number">
<select id="highVolt_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>
</div>

<div class="row">
<div class="col">
<label>Over Load Current</label>
<input id="overLoad" type="number">
<select id="overLoad_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>

<div class="col">
<label>Under Load Current</label>
<input id="underLoad" type="number">
<select id="underLoad_en">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>
</div>
</div>

<!-- ================= POWER RESTORE ================= -->
<div class="section">
<div class="section-title">Power Restore Behaviour</div>
<select id="powerRestore">
<option value="0">Always ON</option>
<option value="1">Stay OFF</option>
<option value="2">Restore Previous</option>
</select>
</div>

<!-- ================= BUZZER ================= -->
<div class="section">
<div class="section-title">Buzzer Settings</div>

<div class="row">
<div class="col">
<label>Buzzer Enable</label>
<select id="buzzerEnable">
<option value="1">Enable</option>
<option value="0">Disable</option>
</select>
</div>

<div class="col">
<label>Manual Test</label>
<button class="btn btn-primary" onclick="testBuzzer()">Activate Buzzer</button>
</div>
</div>
</div>

<!-- ================= ACTION BUTTONS ================= -->
<div class="actions">
<button class="btn btn-danger" onclick="factoryReset()">Factory Reset</button>
<button id="applyBtn" class="btn btn-primary" onclick="apply()">Apply Settings</button>
</div>

<div id="status" class="status">Loading...</div>

</div>
</div>

<script>

window.onload = function(){

  fetch('/settings/get')
  .then(res => res.json())
  .then(data => {

    let ids = [
      "dryRunGap","dryRunGap_en",
      "testingGap","testingGap_en",
      "maxRun","maxRun_en",
      "lowVolt","lowVolt_en",
      "highVolt","highVolt_en",
      "overLoad","overLoad_en",
      "underLoad","underLoad_en",
      "powerRestore",
      "buzzerEnable"
    ];

    ids.forEach(id=>{
      if(data[id] !== undefined){
        document.getElementById(id).value = data[id];
      }
    });

    document.getElementById("status").innerText="Ready";
  })
  .catch(()=>{
    document.getElementById("status").innerText="Failed to load settings";
  });
};


function apply(){

  const btn=document.getElementById("applyBtn");
  const status=document.getElementById("status");

  let keys=[
    "dryRunGap","dryRunGap_en",
    "testingGap","testingGap_en",
    "maxRun","maxRun_en",
    "lowVolt","lowVolt_en",
    "highVolt","highVolt_en",
    "overLoad","overLoad_en",
    "underLoad","underLoad_en",
    "powerRestore",
    "buzzerEnable"
  ];

  let p=[];

  for(let k of keys){
    let el=document.getElementById(k);
    if(!el) continue;
    let val=el.value;
    p.push(k+"="+val);
  }

  btn.disabled=true;
  status.className="status";
  status.innerText="Sending settings...";

  fetch("/settings/set?data="+encodeURIComponent(p.join(";")))
  .then(r=>r.text())
  .then(t=>{
    status.className="status success";
    status.innerText=t;
  })
  .catch(()=>{
    status.className="status error";
    status.innerText="Failed to send settings";
  })
  .finally(()=>btn.disabled=false);
}


function factoryReset(){
  if(confirm("Reset device to factory default?")){
    fetch("/factory_reset")
    .then(()=>alert("Device reset completed"));
  }
}


function testBuzzer(){
  fetch("/buzzer_test")
  .then(r=>r.text())
  .then(t=>alert(t))
  .catch(()=>alert("Buzzer test failed"));
}

</script>

</body>
</html>
)rawliteral";

#endif