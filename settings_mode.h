#ifndef SETTINGS_MODE_H
#define SETTINGS_MODE_H

const char* settingsModeHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Device Settings</title>

<style>
/* ======= SAME STYLING (UNCHANGED) ======= */
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
  --section-bg:#f8fafc;
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
label{font-size:13px;font-weight:700;display:block;margin-bottom:6px;}
input,select{
  width:100%;
  padding:12px;
  border-radius:14px;
  border:1px solid var(--border);
}
.actions{
  display:flex;
  justify-content:space-between;
  margin-top:30px;
  gap:12px;
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

<div class="section">
<div class="section-title">Device Protection</div>
<div class="row">
<div class="col">
<label>Dry Run Gap (minutes)</label>
<input id="dryRunGap" type="number">
</div>
<div class="col">
<label>Testing Gap (minutes)</label>
<input id="testingGap" type="number">
</div>
<div class="col">
<label>Maximum Run Time (minutes)</label>
<input id="maxRun" type="number">
</div>
</div>
</div>

<div class="section">
<div class="section-title">Voltage and Load Limits</div>
<div class="row">
<div class="col">
<label>Low Voltage Cutoff</label>
<input id="lowVolt" type="number">
</div>
<div class="col">
<label>High Voltage Cutoff</label>
<input id="highVolt" type="number">
</div>
</div>
<div class="row">
<div class="col">
<label>Over Load Current</label>
<input id="overLoad" type="number">
</div>
<div class="col">
<label>Under Load Current</label>
<input id="underLoad" type="number">
</div>
</div>
</div>

<div class="section">
<div class="section-title">Power Restore Behaviour</div>
<select id="powerRestore">
<option value="0">Always ON</option>
<option value="1">Stay OFF</option>
<option value="2">Restore Previous</option>
</select>
</div>

<div class="actions">
<button class="btn btn-danger" onclick="factoryReset()">Factory Reset</button>
<button id="applyBtn" class="btn btn-primary" onclick="apply()">Apply Settings</button>
</div>

<div id="status" class="status">Loading...</div>

</div>
</div>

<script>

/* ================= LOAD SAVED SETTINGS ================= */
window.onload = function(){

  fetch('/settings/get')
  .then(res => res.json())
  .then(data => {

    if(data.dryRunGap !== undefined)
      document.getElementById("dryRunGap").value = data.dryRunGap;

    if(data.testingGap !== undefined)
      document.getElementById("testingGap").value = data.testingGap;

    if(data.maxRun !== undefined)
      document.getElementById("maxRun").value = data.maxRun;

    if(data.lowVolt !== undefined)
      document.getElementById("lowVolt").value = data.lowVolt;

    if(data.highVolt !== undefined)
      document.getElementById("highVolt").value = data.highVolt;

    if(data.overLoad !== undefined)
      document.getElementById("overLoad").value = data.overLoad;

    if(data.underLoad !== undefined)
      document.getElementById("underLoad").value = data.underLoad;

    if(data.powerRestore !== undefined)
      document.getElementById("powerRestore").value = data.powerRestore;

    document.getElementById("status").innerText="Ready";

  })
  .catch(()=>{
    document.getElementById("status").innerText="Failed to load settings";
  });
};

/* ================= APPLY ================= */
function apply(){

  const btn=document.getElementById("applyBtn");
  const status=document.getElementById("status");

  let keys=[
    "dryRunGap","testingGap","maxRun",
    "lowVolt","highVolt","overLoad","underLoad",
    "powerRestore"
  ];

  let p=[];

  for(let k of keys){
    let val=document.getElementById(k).value;
    if(val===""){
      status.className="status error";
      status.innerText="All fields required";
      return;
    }
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

/* ================= FACTORY RESET ================= */
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
