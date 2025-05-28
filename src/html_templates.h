// ============================================================================
// html_templates.h - HTML templates with UART display
// ============================================================================
#ifndef HTML_TEMPLATES_H
#define HTML_TEMPLATES_H

const char HTML_MAIN_PAGE[] PROGMEM = R"(
<!DOCTYPE html><html><head><title>ESP32-CAM Honey Badger Detector</title>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<style>
body{font-family:Arial;text-align:center;padding:20px;background:#f4f4f4;margin:0}
.container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1)}
h1{color:#333;margin-bottom:30px}
button{background:#4CAF50;color:white;padding:15px 30px;border:none;border-radius:8px;margin:10px;cursor:pointer;font-size:16px;box-shadow:0 2px 4px rgba(0,0,0,0.2)}
button:hover{background:#45a049;transform:translateY(-2px);transition:0.2s}
button:disabled{background:#ccc;cursor:not-allowed;transform:none}
.test-btn{background:#2196F3;}
.test-btn:hover{background:#1976D2;}
.clear-btn{background:#ff9800;}
.clear-btn:hover{background:#f57c00;}
.uart-btn{background:#673ab7;}
.uart-btn:hover{background:#512da8;}
#status{margin:20px 0;padding:15px;border-radius:8px;font-size:18px;font-weight:bold}
.ready{background:#e7f7e7;color:#2e7d32}
.processing{background:#fff3cd;color:#856404}
.detected{background:#d4edda;color:#155724}
.not-detected{background:#f8d7da;color:#721c24}
.error{background:#f8d7da;color:#721c24}
.info{background:#e8e8e8;color:#333;font-size:14px;margin-top:20px;text-align:left;padding:10px}
.uart-panel{background:#f3e5f5;border:2px solid #673ab7;border-radius:8px;padding:15px;margin:20px 0}
.uart-status{font-weight:bold;margin:10px 0;padding:10px;background:#fff;border-radius:5px}
.uart-command{font-size:24px;font-weight:bold;color:#673ab7}
.debug-log{background:#1e1e1e;color:#00ff00;font-family:monospace;font-size:12px;text-align:left;padding:15px;margin:20px 0;border-radius:8px;max-height:400px;overflow-y:auto;white-space:pre-wrap;word-wrap:break-word}
.log-header{color:#fff;font-weight:bold;margin-bottom:10px}
.timestamp{color:#888}
</style></head>
<body><div class='container'>
<h1>🦡 Honey Badger Detector</h1>
<div style='margin-bottom:20px'>
<button id='detectBtn' onclick='captureAndAnalyze()'>Detect Honey Badger</button>
<button class='test-btn' onclick='testConnection()'>Test Connection</button>
<button class='clear-btn' onclick='clearLog()'>Clear Log</button>
</div>
<div id='status' class='ready'>Ready to capture and analyze</div>

<div class='uart-panel'>
<h3>📡 Arduino UART Communication</h3>
<div class='uart-status' id='uartStatus'>
<div>Status: <span id='uartInitStatus'>Loading...</span></div>
<div>Last Command: <span class='uart-command' id='uartLastCommand'>-</span></div>
<div>Time Since Last: <span id='uartTimeSince'>-</span></div>
</div>
<button class='uart-btn' onclick='testUART()'>Test UART</button>
<div style='margin-top:10px;font-size:12px;color:#666'>
ESP32 TX: Pin {{UART_TX}} | ESP32 RX: Pin {{UART_RX}} | Baud: 115200
</div>
</div>

<div class='info'>
<strong>System Status:</strong><br>
ESP32-CAM IP: {{ESP32_IP}}<br>
Backend: {{BACKEND_INFO}}<br>
WiFi Signal: {{WIFI_SIGNAL}} dBm<br>
Uptime: {{UPTIME}} seconds<br>
Memory: {{MEMORY_INFO}} bytes free
</div>
<div class='debug-log'>
<div class='log-header'>🔍 Debug Log</div>
<div id='logContent'>System ready. Click "Test Connection" to verify backend connectivity.</div>
</div>
</div>
<script>
let isProcessing = false;

function addLog(message, isError = false) {
    const logContent = document.getElementById('logContent');
    const timestamp = new Date().toLocaleTimeString();
    const color = isError ? '#ff4444' : '#00ff00';
    logContent.innerHTML += `<span class='timestamp'>[${timestamp}]</span> <span style='color:${color}'>${message}</span>\n`;
    logContent.scrollTop = logContent.scrollHeight;
}

function clearLog() {
    document.getElementById('logContent').innerHTML = 'Log cleared.\n';
}

async function updateUARTStatus() {
    try {
        const response = await fetch('/uart/status');
        const status = await response.json();
        
        // Show actual device connection status, not just UART initialization
        const isConnected = status.initialized && status.deviceConnected;
        document.getElementById('uartInitStatus').textContent = 
            isConnected ? 'Connected' : (status.initialized ? 'No Arduino' : 'Disconnected');
        document.getElementById('uartInitStatus').style.color = 
            isConnected ? 'green' : 'red';
            
        document.getElementById('uartLastCommand').textContent = 
            status.lastCommand >= 0 ? status.lastCommand : 'None';
            
        const timeSince = status.timeSinceLastResponse || status.timeSinceLastCommand;
        if (timeSince < 60000) {
            document.getElementById('uartTimeSince').textContent = 
                Math.floor(timeSince / 1000) + 's ago';
        } else if (timeSince < 3600000) {
            document.getElementById('uartTimeSince').textContent = 
                Math.floor(timeSince / 60000) + 'm ago';
        } else {
            document.getElementById('uartTimeSince').textContent = 'Long time ago';
        }
        
        // Log new commands only if device is actually connected
        if (isConnected && status.lastCommand >= 0 && status.timeSinceLastCommand < 2000) {
            addLog(`📡 Arduino sent command: ${status.lastCommand}`);
        }
        
    } catch (error) {
        document.getElementById('uartInitStatus').textContent = 'Error';
        document.getElementById('uartInitStatus').style.color = 'red';
    }
}

async function testUART() {
    addLog('📡 Testing UART connection to Arduino...');
    
    try {
        const response = await fetch('/uart/test');
        const result = await response.json();
        
        if (result.success) {
            addLog(`✅ UART test message sent to Arduino`);
        } else {
            addLog(`❌ UART test failed: ${result.error}`, true);
        }
        
    } catch (error) {
        addLog(`❌ UART test request failed: ${error.message}`, true);
    }
}

async function testConnection() {
    addLog('🔗 Testing backend connection...');
    
    try {
        const response = await fetch('/test');
        const result = await response.json();
        
        if (result.success) {
            addLog(`✅ Connection test successful! Response: ${result.responseCode} in ${result.duration}ms`);
            addLog(`📶 WiFi Signal: ${result.wifiSignal} dBm`);
        } else {
            addLog(`❌ Connection test failed: ${result.error}`, true);
            addLog(`📡 Response code: ${result.responseCode}`, true);
        }
        
        addLog(`🎯 Target URL: ${result.testURL}`);
        addLog(`💾 Free heap: ${result.freeHeap} bytes`);
        
    } catch (error) {
        addLog(`❌ Test request failed: ${error.message}`, true);
    }
}

async function captureAndAnalyze(){
    if(isProcessing) return;
    
    isProcessing = true;
    const btn = document.getElementById('detectBtn');
    const status = document.getElementById('status');
    
    btn.disabled = true;
    btn.textContent = 'Processing...';
    status.className = 'processing';
    status.textContent = 'Capturing image and analyzing...';
    
    addLog('📸 Starting honey badger detection...');
    
    try {
        const startTime = Date.now();
        
        addLog('🌐 Sending request to /api/analyze...');
        const response = await fetch('/api/analyze', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'}
        });
        
        addLog(`📡 Received response: ${response.status} ${response.statusText}`);
        
        if (!response.ok) {
            throw new Error(`Server error: ${response.status}`);
        }
        
        const result = await response.json();
        const totalTime = Date.now() - startTime;
        
        addLog(`⏱️ Total request time: ${totalTime}ms`);
        if (result.httpDuration) {
            addLog(`🔗 Backend processing time: ${result.httpDuration}ms`);
        }
        if (result.processingTime) {
            addLog(`⚡ ESP32 processing time: ${result.processingTime}ms`);
        }
        
        if (result.error) {
            addLog(`❌ Analysis error: ${result.error}`, true);
            if (result.code) {
                addLog(`📡 HTTP code: ${result.code}`, true);
            }
            if (result.debug) {
                addLog(`🔍 Debug: ${result.debug}`, true);
            }
            if (result.serverResponse) {
                addLog(`🖥️ Server response: ${result.serverResponse}`, true);
            }
            
            status.className = 'error';
            status.textContent = `Error: ${result.error}`;
        } else if (result.isHoneyBadger) {
            const confidence = (result.confidence * 100).toFixed(1);
            addLog(`🦡 HONEY BADGER DETECTED! Confidence: ${confidence}%`);
            
            status.className = 'detected';
            status.textContent = `🦡 Honey Badger Detected! Confidence: ${confidence}% (${totalTime}ms)`;
        } else {
            const confidence = ((1 - result.confidence) * 100).toFixed(1);
            addLog(`❌ No honey badger found. Confidence: ${confidence}%`);
            
            status.className = 'not-detected';
            status.textContent = `No Honey Badger Found. Confidence: ${confidence}% (${totalTime}ms)`;
        }
        
    } catch (error) {
        addLog(`❌ Request failed: ${error.message}`, true);
        
        status.className = 'error';
        status.textContent = `Connection Error: ${error.message}`;
    } finally {
        isProcessing = false;
        btn.disabled = false;
        btn.textContent = 'Detect Honey Badger';
        
        addLog('✅ Detection process completed');
        
        // Auto-reset status after 10 seconds
        setTimeout(() => {
            if (!isProcessing) {
                status.className = 'ready';
                status.textContent = 'Ready to capture and analyze';
            }
        }, 10000);
    }
}

// Add keyboard shortcut
document.addEventListener('keydown', function(e) {
    if (e.code === 'Space' && !isProcessing) {
        e.preventDefault();
        captureAndAnalyze();
    }
});

// Update UART status every 2 seconds
setInterval(updateUARTStatus, 2000);

// Initial UART status update
setTimeout(updateUARTStatus, 1000);

// Auto-refresh main status every 30 seconds
setInterval(async () => {
    try {
        const response = await fetch('/status');
        const status = await response.json();
        // Update status silently without logging
    } catch (e) {
        // Silent fail
    }
}, 30000);
</script></body></html>
)";

#endif