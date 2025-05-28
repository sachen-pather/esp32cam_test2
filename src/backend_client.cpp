// ============================================================================
// backend_client.cpp - Backend communication implementation
// ============================================================================
#include "backend_client.h"
#include <WiFi.h>

BackendClient::BackendClient() {
  // Constructor - initialization happens per request
}

BackendClient::~BackendClient() {
  // Destructor - cleanup happens per request
}

AnalysisResult BackendClient::analyzeImage(camera_fb_t* fb) {
  AnalysisResult result;
  
  if (!fb) {
    result.error = "Invalid frame buffer";
    return result;
  }
  
  unsigned long startTime = millis();
  result.debug = buildDebugInfo(fb);
  
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(SystemConfig::HTTP_TIMEOUT);
  
  HTTPClient http;
  String backendURL = getBackendURL();
  
  bool httpBeginResult = http.begin(client, backendURL);
  if (!httpBeginResult) {
    result.error = "HTTP client init failed";
    result.debug += "; HTTP client initialization failed";
    return result;
  }
  
  // Create multipart payload
  String boundary = "----ESP32CAMBoundary" + String(millis());
  size_t totalLength;
  uint8_t* payload = createMultipartPayload(fb, boundary, totalLength);
  
  if (!payload) {
    result.error = "Memory allocation failed";
    result.debug += "; Could not allocate " + String(totalLength) + " bytes for payload";
    http.end();
    client.stop();
    return result;
  }
  
  // Set headers
  String contentType = "multipart/form-data; boundary=" + boundary;
  http.addHeader("Content-Type", contentType);
  http.addHeader("User-Agent", "ESP32-CAM-HoneyBadger/2.0");
  http.addHeader("Content-Length", String(totalLength));
  http.setTimeout(SystemConfig::HTTP_TIMEOUT);
  
  result.debug += "; Sending " + String(totalLength) + " bytes to " + backendURL;
  
  // Send request
  unsigned long httpStartTime = millis();
  result.httpCode = http.POST(payload, totalLength);
  result.httpDuration = millis() - httpStartTime;
  
  // Free payload memory
  free(payload);
  
  result.debug += "; HTTP code: " + String(result.httpCode) + 
                 "; Duration: " + String(result.httpDuration) + "ms";
  
  if (result.httpCode == 200) {
    String response = http.getString();
    result.success = true;
    result.serverResponse = response;
    
    // Simple JSON parsing (you might want to use ArduinoJson library for robust parsing)
    // For now, we'll do basic string parsing
    if (response.indexOf("\"isHoneyBadger\":true") != -1) {
      result.isHoneyBadger = true;
    }
    
    // Extract confidence (basic parsing)
    int confIndex = response.indexOf("\"confidence\":");
    if (confIndex != -1) {
      int startIndex = confIndex + 13; // Length of "\"confidence\":"
      int endIndex = response.indexOf(",", startIndex);
      if (endIndex == -1) endIndex = response.indexOf("}", startIndex);
      
      if (endIndex > startIndex) {
        String confStr = response.substring(startIndex, endIndex);
        result.confidence = confStr.toFloat();
      }
    }
    
    result.debug += "; Success! Response length: " + String(response.length());
    
  } else if (result.httpCode > 0) {
    String errorBody = http.getString();
    result.error = "Backend HTTP error";
    result.serverResponse = errorBody;
    result.debug += "; Error body: " + errorBody.substring(0, 100); // First 100 chars
    
  } else {
    String errorString = http.errorToString(result.httpCode);
    result.error = "Connection failed: " + errorString;
    result.debug += "; " + errorString + "; WiFi status: " + String(WiFi.status()) + 
                   "; Signal: " + String(WiFi.RSSI()) + " dBm";
  }
  
  result.processingTime = millis() - startTime;
  result.debug += "; Total time: " + String(result.processingTime) + "ms";
  
  http.end();
  client.stop();
  
  return result;
}

ConnectionTestResult BackendClient::testConnection() {
  ConnectionTestResult result;
  result.testURL = "https://" + String(NetworkConfig::BACKEND_HOST) + ":" + String(NetworkConfig::BACKEND_PORT) + "/";
  result.wifiSignal = WiFi.RSSI();
  result.freeHeap = ESP.getFreeHeap();
  
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(10000);
  
  HTTPClient http;
  bool httpBeginResult = http.begin(client, result.testURL);
  
  if (!httpBeginResult) {
    result.error = "HTTP client init failed";
    return result;
  }
  
  http.setTimeout(10000);
  unsigned long startTime = millis();
  result.responseCode = http.GET();
  result.duration = millis() - startTime;
  
  if (result.responseCode > 0) {
    result.success = true;
  } else {
    result.error = http.errorToString(result.responseCode);
  }
  
  http.end();
  client.stop();
  
  return result;
}

String BackendClient::getBackendURL() {
  return "https://" + String(NetworkConfig::BACKEND_HOST) + NetworkConfig::ANALYZE_ENDPOINT;
}

String BackendClient::buildDebugInfo(camera_fb_t* fb) {
  String info = "WiFi Signal: " + String(WiFi.RSSI()) + " dBm";
  info += "; Free heap before: " + String(ESP.getFreeHeap()) + " bytes";
  info += "; Image: " + String(fb->width) + "x" + String(fb->height);
  info += " (" + String(fb->len) + " bytes)";
  return info;
}

uint8_t* BackendClient::createMultipartPayload(camera_fb_t* fb, const String& boundary, size_t& totalLength) {
  String bodyStart = "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n";
  bodyStart += "Content-Type: image/jpeg\r\n\r\n";
  
  String bodyEnd = "\r\n--" + boundary + "--\r\n";
  
  totalLength = bodyStart.length() + fb->len + bodyEnd.length();
  
  uint8_t* payload = (uint8_t*)malloc(totalLength);
  if (!payload) {
    return nullptr;
  }
  
  size_t offset = 0;
  memcpy(payload + offset, bodyStart.c_str(), bodyStart.length());
  offset += bodyStart.length();
  memcpy(payload + offset, fb->buf, fb->len);
  offset += fb->len;
  memcpy(payload + offset, bodyEnd.c_str(), bodyEnd.length());
  
  return payload;
}