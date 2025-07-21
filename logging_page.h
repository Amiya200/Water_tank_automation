#pragma once
const char* loggingPageHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>System Logs - Water Tank Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: 'Segoe UI', Roboto, sans-serif;
    }
    
    body {
      background-color: #f5f7fa;
      color: #333;
    }
    
    .container {
      max-width: 1000px;
      margin: 0 auto;
      padding: 20px;
    }
    
    header {
      background-color: #2c3e50;
      color: white;
      padding: 20px 0;
      text-align: center;
      margin-bottom: 30px;
      border-radius: 8px;
    }
    
    h1 {
      font-size: 28px;
      margin-bottom: 10px;
    }
    
    .log-container {
      background: white;
      border-radius: 8px;
      padding: 20px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      margin-bottom: 30px;
    }
    
    .log-controls {
      display: flex;
      justify-content: space-between;
      margin-bottom: 20px;
      gap: 10px;
      flex-wrap: wrap;
    }
    
    .btn {
      padding: 10px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-weight: 600;
      transition: all 0.3s;
    }
    
    .btn-primary {
      background-color: #3498db;
      color: white;
    }
    
    .btn-primary:hover {
      background-color: #2980b9;
    }
    
    .btn-danger {
      background-color: #e74c3c;
      color: white;
    }
    
    .btn-danger:hover {
      background-color: #c0392b;
    }
    
    .log-display {
      background-color: #1a1a1a;
      color: #e0e0e0;
      padding: 15px;
      border-radius: 4px;
      font-family: 'Courier New', monospace;
      font-size: 14px;
      height: 500px;
      overflow-y: auto;
      white-space: pre-wrap;
    }
    
    .log-entry {
      margin-bottom: 8px;
      padding-bottom: 8px;
      border-bottom: 1px solid #333;
    }
    
    .log-time {
      color: #95a5a6;
      margin-right: 10px;
    }
    
    .log-message {
      color: #ffffff;
    }
    
    .log-error {
      color: #e74c3c;
    }
    
    .log-warning {
      color: #f39c12;
    }
    
    .log-info {
      color: #3498db;
    }
    
    .footer {
      text-align: center;
      margin-top: 30px;
      padding-top: 20px;
      border-top: 1px solid #eee;
      color: #7f8c8d;
    }
    
    @media (max-width: 768px) {
      .log-controls {
        flex-direction: column;
      }
    }
  </style>
</head>
<body>
  <header>
    <div class="container">
      <h1>Water Tank Controller System Logs</h1>
      <p>Monitoring system events and operations</p>
    </div>
  </header>
  
  <div class="container">
    <div class="log-container">
      <div class="log-controls">
        <div>
          <button id="refreshBtn" class="btn btn-primary">Refresh Logs</button>
          <button id="clearBtn" class="btn btn-primary">Clear Display</button>
        </div>
        <button id="backBtn" class="btn btn-danger">Back to Control Panel</button>
      </div>
      
      <div id="logDisplay" class="log-display">
        Loading logs... Please wait.
      </div>
    </div>
    
    <div class="footer">
      <p>Halonix Water Tank Controller &copy; 2023 | System Version 2.1.4</p>
      <p>Last updated: <span id="lastUpdated">Not available</span></p>
    </div>
  </div>

  <script>
    // DOM elements
    const logDisplay = document.getElementById('logDisplay');
    const refreshBtn = document.getElementById('refreshBtn');
    const clearBtn = document.getElementById('clearBtn');
    const backBtn = document.getElementById('backBtn');
    const lastUpdated = document.getElementById('lastUpdated');
    
    // Log buffer to prevent duplicates
    let lastLogCount = 0;
    
    // Format a timestamp
    function formatTimestamp(timestamp) {
      const date = new Date(timestamp * 1000);
      return date.toLocaleTimeString();
    }
    
    // Append log to display
    function appendLog(message, timestamp, type = 'message') {
      const logClass = `log-${type}`;
      const timeStr = timestamp ? `[${formatTimestamp(timestamp)}]` : '[System]';
      
      const logEntry = document.createElement('div');
      logEntry.className = 'log-entry';
      logEntry.innerHTML = `<span class="log-time">${timeStr}</span>
                           <span class="${logClass}">${message}</span>`;
      
      logDisplay.appendChild(logEntry);
      
      // Auto-scroll to bottom
      logDisplay.scrollTop = logDisplay.scrollHeight;
    }
    
    // Clear log display
    function clearLogDisplay() {
      logDisplay.innerHTML = '';
      appendLog('Log display cleared', null, 'info');
    }
    
    // Fetch logs from server
    async function fetchLogs() {
      try {
        const response = await fetch('/get_logs');
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        const logs = await response.json();
        
        // Only update if we have new logs
        if (logs.length !== lastLogCount) {
          if (logs.length > lastLogCount) {
            // Only append new logs
            for (let i = lastLogCount; i < logs.length; i++) {
              appendLog(logs[i].message, logs[i].timestamp, logs[i].type);
            }
          } else {
            // Full refresh if logs were cleared on server
            logDisplay.innerHTML = '';
            logs.forEach(log => {
              appendLog(log.message, log.timestamp, log.type);
            });
          }
          
          lastLogCount = logs.length;
          updateLastUpdated();
        }
      } catch (error) {
        console.error('Error fetching logs:', error);
        appendLog(`Error fetching logs: ${error.message}`, null, 'error');
      }
    }
    
    // Update last updated time
    function updateLastUpdated() {
      const now = new Date();
      lastUpdated.textContent = now.toLocaleString();
    }
    
    // Initialize
    async function init() {
      // Event listeners
      refreshBtn.addEventListener('click', fetchLogs);
      clearBtn.addEventListener('click', clearLogDisplay);
      backBtn.addEventListener('click', () => {
        window.location.href = '/';
      });
      
      // Initial fetch
      await fetchLogs();
      
      // Auto-refresh every 5 seconds
      setInterval(fetchLogs, 5000);
    }
    
    // Start the app
    init();
  </script>
</body>
</html>
)rawliteral";
