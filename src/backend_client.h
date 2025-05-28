// ============================================================================
// backend_client.h - Backend communication declarations
// ============================================================================
#ifndef BACKEND_CLIENT_H
#define BACKEND_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "config.h"

struct AnalysisResult {
  bool success;
  bool isHoneyBadger;
  float confidence;
  String error;
  String debug;
  String serverResponse;
  unsigned long processingTime;
  unsigned long httpDuration;
  int httpCode;
  
  // Constructor for easy initialization
  AnalysisResult() : success(false), isHoneyBadger(false), confidence(0.0), 
                    processingTime(0), httpDuration(0), httpCode(0) {}
};

struct ConnectionTestResult {
  bool success;
  int responseCode;
  unsigned long duration;
  String error;
  String testURL;
  int wifiSignal;
  size_t freeHeap;
  
  // Constructor
  ConnectionTestResult() : success(false), responseCode(0), duration(0), 
                          wifiSignal(0), freeHeap(0) {}
};

class BackendClient {
private:
  String buildDebugInfo(camera_fb_t* fb);
  uint8_t* createMultipartPayload(camera_fb_t* fb, const String& boundary, size_t& totalLength);
  
public:
  BackendClient();
  ~BackendClient();
  
  // Main functionality
  AnalysisResult analyzeImage(camera_fb_t* fb);
  ConnectionTestResult testConnection();
  
  // Utility
  String getBackendURL();
};

#endif