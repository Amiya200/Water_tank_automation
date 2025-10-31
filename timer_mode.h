#ifndef TIMER_MODE_H
#define TIMER_MODE_H

const char* timerModeHtml = R"rawliteral(
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
      display: block;
      margin-bottom: 5px;
      color: #ccc;
    }
    .custom-time {
      width: 45%;
      padding: 12px;
      margin: 8px;
      font-size: 16px;
      border-radius: 6px;
      border: 2px solid #333;
      background: #222;
      color: white;
      cursor: pointer;
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
    .success { color: #28a745; border: 2px solid #28a745; }
    .error   { color: #dc3545; border: 2px solid #dc3545; }
    .loading { font-size: 14px; color: #0dcaf0; margin-top: 10px; font-style: italic; }
    .back-bottom {
      background: #222; margin-top: 18px; border: 1px solid #0dcaf0;
      color: #0dcaf0; padding: 10px 20px; border-radius: 6px; cursor: pointer;
    }
    .optional { font-size:12px;color:#888;font-style:italic;margin-top:5px; }

    /* --- Modal --- */
    .modal {
      display:none;
      position:fixed;
      z-index:1000;
      left:0; top:0;
      width:100%; height:100%;
      background:rgba(0,0,0,0.7);
      justify-content:center;
      align-items:center;
    }
    .modal-content {
      background:#111;
      padding:20px;
      border-radius:12px;
      text-align:center;
      width:280px;
      box-shadow:0 0 10px #0dcaf0;
    }
    .modal-content h2 {
      color:#0dcaf0;
      margin-bottom:15px;
    }
    .time-inputs {
      display:flex;
      justify-content:center;
      align-items:center;
      gap:10px;
      margin-bottom:20px;
    }
    select {
      background:#222;
      color:white;
      border:2px solid #333;
      border-radius:6px;
      font-size:18px;
      padding:5px;
    }
    .modal-btn {
      padding:10px 20px;
      margin:5px;
      border:none;
      border-radius:8px;
      font-weight:bold;
      cursor:pointer;
      font-size:15px;
    }
    .set-btn { background:#0dcaf0; color:white; }
    .cancel-btn { background:#333; color:#ccc; border:1px solid #555; }
  </style>
</head>
<body>
  <h1>Set Timer Mode</h1>

  <div class="form-container">
    <form id="timerForm">
      <div id="slotsContainer"></div>
      <button type="submit">Set Timer</button>
    </form>

    <div class="loading" id="loading" style="display: none;">Sending timer configuration...</div>
    <div id="result" class="result"></div>
    <button class="back-bottom" type="button" onclick="window.location.href='/'">Back to Home</button>
  </div>

  <!-- Custom Time Picker Modal -->
  <div class="modal" id="timeModal">
    <div class="modal-content">
      <h2>Select Time</h2>
      <div class="time-inputs">
        <select id="hourSelect"></select> :
        <select id="minuteSelect"></select>
      </div>
      <button class="modal-btn set-btn" id="setBtn">Set</button>
      <button class="modal-btn cancel-btn" id="cancelBtn">Cancel</button>
    </div>
  </div>

  <script>
    const slotsContainer = document.getElementById('slotsContainer');
    const timeModal = document.getElementById('timeModal');
    const hourSel = document.getElementById('hourSelect');
    const minSel = document.getElementById('minuteSelect');
    const setBtn = document.getElementById('setBtn');
    const cancelBtn = document.getElementById('cancelBtn');

    // populate hours/minutes
    for (let h=0; h<24; h++) {
      let opt=document.createElement('option');
      opt.value=opt.textContent=String(h).padStart(2,'0');
      hourSel.appendChild(opt);
    }
    for (let m=0; m<60; m+=1) {
      let opt=document.createElement('option');
      opt.value=opt.textContent=String(m).padStart(2,'0');
      minSel.appendChild(opt);
    }

    let currentInput=null;

    // build slots
    for (let i=1;i<=5;i++){
      const slot=document.createElement('div');
      slot.className='time-slot';
      slot.innerHTML=`
        <h3>Time Slot ${i}</h3>
        <label>ON Time:</label>
        <input class="custom-time" id="onTime${i}" readonly placeholder="Select time">
        <label>OFF Time:</label>
        <input class="custom-time" id="offTime${i}" readonly placeholder="Select time">
        <div class="optional">(Optional - fill both or none)</div>`;
      slotsContainer.appendChild(slot);
    }

    document.querySelectorAll('.custom-time').forEach(inp=>{
      inp.addEventListener('click',()=>{
        currentInput=inp;
        const val=inp.value.split(':');
        if(val.length===2){hourSel.value=val[0];minSel.value=val[1];}
        timeModal.style.display='flex';
      });
    });

    setBtn.onclick=()=>{
      if(currentInput){
        const t=`${hourSel.value}:${minSel.value}`;
        currentInput.value=t;
      }
      timeModal.style.display='none';
    };
    cancelBtn.onclick=()=>{timeModal.style.display='none';};

    window.onclick=(e)=>{if(e.target===timeModal)timeModal.style.display='none';};

    // form submission
    document.getElementById('timerForm').addEventListener('submit',function(e){
      e.preventDefault();
      const result=document.getElementById('result');
      const loading=document.getElementById('loading');
      const times=[];
      for(let i=1;i<=5;i++){
        const on=document.getElementById('onTime'+i).value;
        const off=document.getElementById('offTime'+i).value;
        if((on&&!off)||(!on&&off)){
          result.className='result error';
          result.innerHTML=`Incomplete slot ${i}. Fill both or none.`;
          return;
        }
        if(on&&off){
          times.push(`on${i}=${encodeURIComponent(on)}&off${i}=${encodeURIComponent(off)}`);
        }
      }
      if(times.length===0){
        result.className='result error';
        result.innerHTML='Please fill at least one complete time slot.';
        return;
      }
      loading.style.display='block';
      result.innerHTML='';
      fetch('/timer/set?'+times.join('&'))
        .then(r=>{loading.style.display='none';if(r.ok)return r.text();throw new Error('Server '+r.status);})
        .then(t=>{
          result.className='result success';
          result.innerHTML='Timer configuration saved successfully!<br><br><pre style="text-align:left;white-space:pre-wrap;">'+t+'</pre>';
        })
        .catch(e=>{
          loading.style.display='none';
          result.className='result error';
          result.innerHTML='Connection error: '+e.message;
        });
    });
  </script>
</body>
</html>
)rawliteral";

#endif // TIMER_MODE_H
