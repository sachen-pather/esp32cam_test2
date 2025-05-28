// ============================================================================
// web_server.cpp - Web server implementation (WITH UART)
// ============================================================================
#include "web_server.h"
#include "html_templates.h"
#include "config.h"

WebServerManager::WebServerManager(WebServer* srv, CameraModule* cam, WiFiModule* wf, UARTController* uart) 
  : server(srv), camera(cam), wifi(wf), uartController(uart) {
}

void WebServerManager::setupRoutes() {
  // Existing routes
  server->on("/", HTTP_GET, [this]() { handleRoot(); });
  server->on("/capture", HTTP_GET, [this]() { handleCapture(); });
  server->on("/api/analyze", HTTP_GET, [this]() { handleAnalyzeAPI(); });
  server->on("/api/analyze", HTTP_POST, [this]() { handleAnalyzeAPI(); });
  server->on("/status", HTTP_GET, [this]() { handleStatus(); });
  server->on("/test", HTTP_GET, [this]() { handleTestConnection(); });
  
  // NEW: UART control routes
  server->on("/uart/status", HTTP_GET, [this]() { handleUARTStatus(); });
  server->on("/uart/test", HTTP_GET, [this]() { handleUARTTest(); });
  
  server->onNotFound([this]() { handleNotFound(); });
  
  Serial.println("Web server routes configured (with UART control)");
}

void WebServerManager::handleClient() {
  server->handleClient();
}

void WebServerManager::handleRoot() {
  String html = processHTMLTemplate(FPSTR(HTML_MAIN_PAGE));
  server->send(200, "text/html", html);
}

void WebServerManager::handleCapture() {
  if (!camera->isInitialized()) {
    server->send(503, "text/plain", "Camera not available");
    return;
  }
  
  camera_fb_t* fb = camera->captureImage();
  if (!fb) {
    server->send(500, "text/plain", "Capture failed");
    return;
  }
  
  sendImageResponse(fb);
  camera->releaseFrameBuffer(fb);
}

void WebServerManager::handleAnalyzeAPI() {
  // Check system status
  if (!camera->isInitialized()) {
    server->send(503, "application/json", 
      "{\"error\":\"Camera not initialized\",\"debug\":\"Camera initialization failed during startup\"}");
    return;
  }
  
  if (!wifi->isConnected()) {
    server->send(503, "application/json", 
      "{\"error\":\"WiFi not connected\",\"debug\":\"WiFi connection lost or never established\"}");
    return;
  }
  
  // Capture image
  camera_fb_t* fb = camera->captureImage();
  if (!fb) {
    server->send(500, "application/json", 
      "{\"error\":\"Camera capture failed\",\"debug\":\"Frame buffer allocation failed - possible memory issue\"}");
    return;
  }
  
  // Analyze with backend
  AnalysisResult result = backendClient.analyzeImage(fb);
  camera->releaseFrameBuffer(fb);
  
  // Build response JSON
  String response = "{";
  if (result.success) {
    response += "\"isHoneyBadger\":" + String(result.isHoneyBadger ? "true" : "false") + ",";
    response += "\"confidence\":" + String(result.confidence) + ",";
    response += "\"processingTime\":" + String(result.processingTime) + ",";
    response += "\"httpDuration\":" + String(result.httpDuration) + ",";
    response += "\"captureTime\":\"" + String(millis()) + "\",";
    response += "\"debug\":\"" + result.debug + "\"";
  } else {
    response += "\"error\":\"" + result.error + "\",";
    response += "\"code\":" + String(result.httpCode) + ",";
    response += "\"debug\":\"" + result.debug + "\"";
    if (result.serverResponse.length() > 0 && result.serverResponse.length() < 200) {
      String escapedResponse = result.serverResponse;
      escapedResponse.replace("\"", "'");
      response += ",\"serverResponse\":\"" + escapedResponse + "\"";
    }
  }
  response += "}";
  
  server->send(result.success ? 200 : 500, "application/json", response);
}

void WebServerManager::handleStatus() {
  String json = getStatusJSON();
  server->send(200, "application/json", json);
}

void WebServerManager::handleTestConnection() {
  if (!wifi->isConnected()) {
    String response = "{\"error\":\"WiFi not connected\",\"wifiStatus\":" + String(WiFi.status()) + "}";
    server->send(503, "application/json", response);
    return;
  }
  
  ConnectionTestResult testResult = backendClient.testConnection();
  
  String response = "{";
  response += "\"testURL\":\"" + testResult.testURL + "\",";
  response += "\"responseCode\":" + String(testResult.responseCode) + ",";
  response += "\"duration\":" + String(testResult.duration) + ",";
  response += "\"success\":" + String(testResult.success ? "true" : "false") + ",";
  response += "\"wifiSignal\":" + String(testResult.wifiSignal) + ",";
  response += "\"freeHeap\":" + String(testResult.freeHeap);
  
  if (!testResult.success) {
    response += ",\"error\":\"" + testResult.error + "\"";
  }
  
  response += "}";
  
  server->send(200, "application/json", response);
}

// NEW: UART Status Handler
void WebServerManager::handleUARTStatus() {
  if (!uartController || !uartController->isInitialized()) {
    server->send(503, "application/json", "{\"error\":\"UART controller not available\"}");
    return;
  }
  
  String json = "{";
  json += "\"initialized\":" + String(uartController->isInitialized() ? "true" : "false") + ",";
  json += "\"deviceConnected\":" + String(uartController->isDeviceConnected() ? "true" : "false") + ",";  // NEW
  json += "\"lastCommand\":" + String(uartController->getLastCommand()) + ",";
  json += "\"lastCommandTime\":" + String(uartController->getLastCommandTime()) + ",";
  json += "\"lastResponseTime\":" + String(uartController->getLastResponseTime()) + ",";  // NEW
  json += "\"timeSinceLastCommand\":" + String(millis() - uartController->getLastCommandTime()) + ",";
  json += "\"timeSinceLastResponse\":" + String(millis() - uartController->getLastResponseTime()) + ",";  // NEW
  json += "\"txPin\":" + String(SystemPins::UART_TX) + ",";
  json += "\"rxPin\":" + String(SystemPins::UART_RX) + ",";
  json += "\"baudRate\":" + String(UARTConfig::BAUD_RATE);
  json += "}";
  
  server->send(200, "application/json", json);
}

// NEW: UART Test Handler
void WebServerManager::handleUARTTest() {
  if (!uartController || !uartController->isInitialized()) {
    server->send(503, "application/json", "{\"error\":\"UART controller not available\"}");
    return;
  }
  
  uartController->pingDevice();  // Send ping to test actual connection
  
  String response = "{";
  response += "\"success\":true,";
  response += "\"message\":\"Ping sent to Arduino - check if device responds\",";
  response += "\"timestamp\":" + String(millis()) + ",";
  response += "\"deviceConnected\":" + String(uartController->isDeviceConnected() ? "true" : "false");
  response += "}";
  
  server->send(200, "application/json", response);
}

void WebServerManager::handleNotFound() {
  server->send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
}

String WebServerManager::getStatusJSON() {
  String json = "{";
  json += "\"camera\":" + String(camera->isInitialized() ? "true" : "false") + ",";
  json += "\"wifi\":" + String(wifi->isConnected() ? "true" : "false") + ",";
  json += "\"wifiStatus\":" + String(WiFi.status()) + ",";
  json += "\"wifiSignal\":" + String(wifi->getSignalStrength()) + ",";
  json += "\"ip\":\"" + wifi->getIP() + "\",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"freePSRAM\":" + String(ESP.getFreePsram()) + ",";
  json += "\"uptime\":" + String(millis()) + ",";
  json += "\"backend\":\"https://" + String(NetworkConfig::BACKEND_HOST) + ":" + String(NetworkConfig::BACKEND_PORT) + "\"";
  
  // Add UART status
  if (uartController && uartController->isInitialized()) {
    json += ",\"uart\":{";
    json += "\"initialized\":true,";
    json += "\"lastCommand\":" + String(uartController->getLastCommand()) + ",";
    json += "\"timeSinceLastCommand\":" + String(millis() - uartController->getLastCommandTime());
    json += "}";
  } else {
    json += ",\"uart\":{\"initialized\":false}";
  }
  
  json += "}";
  return json;
}

String WebServerManager::processHTMLTemplate(const String& html) {
  String processed = html;
  
  // Replace template variables
  processed.replace("{{ESP32_IP}}", wifi->getIP());
  processed.replace("{{BACKEND_INFO}}", String(NetworkConfig::BACKEND_HOST) + ":" + String(NetworkConfig::BACKEND_PORT));
  processed.replace("{{MEMORY_INFO}}", String(ESP.getFreeHeap()));
  processed.replace("{{WIFI_SIGNAL}}", String(wifi->getSignalStrength()));
  processed.replace("{{UPTIME}}", String(millis() / 1000));
  
  // NEW: Replace UART pin info
  processed.replace("{{UART_TX}}", String(SystemPins::UART_TX));
  processed.replace("{{UART_RX}}", String(SystemPins::UART_RX));
  
  return processed;
}

void WebServerManager::sendImageResponse(camera_fb_t* fb) {
  // Send headers
  server->sendHeader("Content-Type", "image/jpeg");
  server->sendHeader("Content-Length", String(fb->len));
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Content-Disposition", "inline; filename=esp32cam.jpg");
  
  // Send image data in chunks
  WiFiClient client = server->client();
  size_t sent = 0;
  
  while (sent < fb->len) {
    size_t toSend = min((size_t)SystemConfig::CHUNK_SIZE, fb->len - sent);
    client.write(fb->buf + sent, toSend);
    sent += toSend;
  }
  
  Serial.printf("Image sent: %u bytes\n", fb->len);
}