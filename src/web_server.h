// ============================================================================
// web_server.h - Web server functionality declarations (WITH UART)
// ============================================================================
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include "camera_module.h"
#include "wifi_module.h"
#include "backend_client.h"
#include "uart_controller.h"  // NEW: UART controller

class WebServerManager {
private:
  WebServer* server;
  CameraModule* camera;
  WiFiModule* wifi;
  UARTController* uartController;  // NEW: UART controller pointer
  BackendClient backendClient;
  
  // Route handlers
  void handleRoot();
  void handleCapture();
  void handleAnalyzeAPI();
  void handleStatus();
  void handleTestConnection();
  void handleNotFound();
  
  // NEW: UART control handlers
  void handleUARTStatus();
  void handleUARTTest();
  
  // Utility functions
  String getStatusJSON();
  String processHTMLTemplate(const String& html);
  void sendImageResponse(camera_fb_t* fb);
  
public:
  WebServerManager(WebServer* srv, CameraModule* cam, WiFiModule* wf, UARTController* uart);
  
  void setupRoutes();
  void handleClient();
};

#endif