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

  --section-bg:#f8fafc;
}

*{box-sizing:border-box}

body{
  margin:0;
  font-family:Inter,Segoe UI,Roboto,Arial,sans-serif;
  background:linear-gradient(135deg,var(--bg1),var(--bg2));
  color:var(--text);
}

.wrap{
  max-width:920px;
  margin:32px auto;
  padding:16px;
}

.card{
  background:linear-gradient(180deg,#ffffff,#f9fafb);
  border-radius:22px;
  padding:26px;
  box-shadow:
    0 20px 40px rgba(0,0,0,.08),
    0 1px 0 rgba(255,255,255,.8) inset;
}

/* HEADER */
.header{
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-bottom:22px;
}

.header h2{
  margin:0;
  font-size:24px;
  font-weight:800;
  background:linear-gradient(135deg,var(--primary),var(--primary2));
  -webkit-background-clip:text;
  -webkit-text-fill-color:transparent;
}

.subtitle{
  font-size:13px;
  color:var(--muted);
  margin-top:4px;
}

.back-btn button{
  background:linear-gradient(135deg,#ffffff,#f1f5f9);
  border:1px solid var(--border);
  padding:9px 16px;
  border-radius:12px;
  font-weight:700;
  cursor:pointer;
  color:#334155;
  transition:.2s;
}
.back-btn button:hover{
  box-shadow:0 6px 14px rgba(0,0,0,.08);
}

/* SECTIONS */
.section{
  margin-top:30px;
  padding:22px;
  border-radius:18px;
  background:linear-gradient(180deg,var(--section-bg),#ffffff);
  border:1px solid var(--border);
}

.section-title{
  font-size:16px;
  font-weight:800;
  margin-bottom:16px;
  color:#1e293b;
  position:relative;
}
.section-title::after{
  content:'';
  position:absolute;
  left:0;
  bottom:-6px;
  width:40px;
  height:3px;
  border-radius:3px;
  background:linear-gradient(135deg,var(--primary),var(--primary2));
}

/* GRID */
.row{
  display:flex;
  gap:18px;
  flex-wrap:wrap;
}

.col{
  flex:1;
  min-width:180px;
}

label{
  font-size:13px;
  font-weight:700;
  display:block;
  margin-bottom:6px;
  color:#334155;
}

/* INPUTS */
input,select{
  width:100%;
  padding:12px;
  border-radius:14px;
  border:1px solid var(--border);
  font-size:14px;
  background:#ffffff;
  transition:.2s;
}

input::placeholder{color:#9ca3af}

input:focus,select:focus{
  outline:none;
  border-color:var(--primary);
  box-shadow:0 0 0 4px rgba(124,58,237,.18);
}

.helper{
  font-size:12px;
  color:var(--muted);
  margin-top:4px;
}

/* ACTIONS */
.actions{
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-top:34px;
  gap:14px;
}

.btn{
  padding:14px 26px;
  border-radius:16px;
  border:none;
  font-weight:800;
  font-size:15px;
  cursor:pointer;
  transition:.25s;
}

.btn-primary{
  background:linear-gradient(135deg,var(--primary),var(--primary2));
  color:#ffffff;
  box-shadow:0 10px 20px rgba(124,58,237,.35);
}
.btn-primary:hover{
  transform:translateY(-2px);
  box-shadow:0 14px 26px rgba(124,58,237,.45);
}
.btn-primary:disabled{
  opacity:.6;
  transform:none;
  box-shadow:none;
}

.btn-danger{
  background:#fff;
  border:2px solid var(--danger);
  color:var(--danger);
}

.btn-secondary{
  background:#f8fafc;
  border:1px solid var(--border);
  color:#334155;
}

/* STATUS */
.status{
  margin-top:22px;
  padding:16px;
  border-radius:16px;
  font-size:14px;
  border:1px solid var(--border);
  background:#ffffff;
}

.status.success{
  background:linear-gradient(135deg,#ecfdf5,#f0fdf4);
  border-color:#bbf7d0;
  color:var(--success);
}

.status.error{
  background:linear-gradient(135deg,#fef2f2,#fff1f2);
  border-color:#fecaca;
  color:var(--danger);
}

/* MOBILE */
@media (max-width:640px){
  .actions{
    flex-direction:column;
    align-items:stretch;
  }
}
</style>
</head>

<body>
<div class="wrap">
<div class="card">

<div class="header">
  <div>
    <h2>Device Settings</h2>
    <div class="subtitle">Protection thresholds and power behaviour</div>
  </div>
  <div class="back-btn">
    <button onclick="location.href='/'">Back</button>
  </div>
</div>

<!-- DEVICE PROTECTION -->
<div class="section">
  <div class="section-title">Device Protection</div>
  <div class="row">
    <div class="col">
      <label>Dry Run Gap (minutes)</label>
      <input id="dryRunGap" type="number" min="0" max="15" placeholder="0 to 15">
      <div class="helper">Set 0 to disable dry run protection</div>
    </div>
    <div class="col">
      <label>Testing Gap (minutes)</label>
      <input id="testingGap" type="number" min="0" max="180" placeholder="0 to 180">
    </div>
    <div class="col">
      <label>Maximum Run Time (minutes)</label>
      <input id="maxRun" type="number" min="0" max="300" placeholder="0 to 300">
    </div>
  </div>
</div>

<!-- VOLTAGE & LOAD -->
<div class="section">
  <div class="section-title">Voltage and Load Limits</div>
  <div class="row">
    <div class="col">
      <label>Low Voltage Cutoff (V)</label>
      <input id="lowVolt" type="number" placeholder="e.g. 190">
    </div>
    <div class="col">
      <label>High Voltage Cutoff (V)</label>
      <input id="highVolt" type="number" placeholder="e.g. 270">
    </div>
  </div>
  <div class="row">
    <div class="col">
      <label>Over Load Current (A)</label>
      <input id="overLoad" type="number" placeholder="e.g. 10">
    </div>
    <div class="col">
      <label>Under Load Current (A)</label>
      <input id="underLoad" type="number" placeholder="e.g. 5">
    </div>
  </div>
</div>

<!-- POWER RESTORE -->
<div class="section">
  <div class="section-title">Power Restore Behaviour</div>
  <select id="powerRestore">
    <option value="0">Always ON after power restore</option>
    <option value="1">Stay OFF after power restore</option>
    <option value="2">Restore previous state</option>
  </select>
</div>

<!-- ACTIONS -->
<div class="actions">
  <button class="btn btn-secondary" onclick="location.href='/'">Back</button>
  <button class="btn btn-danger" onclick="factoryReset()">Factory Reset</button>
  <button id="applyBtn" class="btn btn-primary" onclick="apply()">Apply Settings</button>
</div>

<div id="status" class="status">Ready</div>

</div>
</div>

<script>
function apply(){
  const btn=document.getElementById("applyBtn");
  const status=document.getElementById("status");

  let p=[];
  const keys=[
    "dryRunGap","testingGap","maxRun",
    "lowVolt","highVolt","overLoad","underLoad",
    "powerRestore"
  ];

  for(let k of keys){
    let el=document.getElementById(k);
    if(!el || el.value===""){
      status.className="status error";
      status.innerText="All fields must be filled before applying settings.";
      return;
    }
    p.push(k+"="+el.value);
  }

  btn.disabled=true;
  status.className="status";
  status.innerText="Sending settings to device...";

  fetch("/settings/set?data="+encodeURIComponent(p.join(":")))
    .then(r=>r.text())
    .then(t=>{
      status.className="status success";
      status.innerText=t;
    })
    .catch(()=>{
      status.className="status error";
      status.innerText="Failed to send settings to device.";
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
