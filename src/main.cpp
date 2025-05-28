// ============================================================================
// main.cpp - Main application file (WITH UART)
// ============================================================================
#include <Arduino.h>
#include <WebServer.h>
#include "config.h"
#include "system_utils.h"
#include "camera_module.h"
#include "wifi_module.h"
#include "web_server.h"
#include "uart_controller.h"  // NEW: UART controller

// Global objects
CameraModule camera;
WiFiModule wifiModule;
UARTController uartController;  // NEW: UART controller instance
WebServer server(SystemConfig::WEB_SERVER_PORT);
WebServerManager webManager(&server, &camera, &wifiModule, &uartController);  // Pass UART controller

// Timing variables
unsigned long lastHeartbeat = 0;
unsigned long lastWifiCheck = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(SystemConfig::SERIAL_BAUD);
  delay(1000);
  
  // Print startup header
  SystemUtils::printHeader();
  
  // Initialize system components
  SystemUtils::initializePins();
  SystemUtils::printSystemInfo();
  
  // Initialize UART controller (NEW)
  if (!uartController.initialize()) {
    Serial.println("WARNING: UART controller failed to initialize");
  }
  
  // Initialize camera (critical component)
  if (!camera.initialize()) {
    Serial.println("CRITICAL: Camera failed to initialize. System halted.");
    while (true) {
      SystemUtils::blinkError(1);
      delay(1000);
    }
  }
  
  // Initialize WiFi
  bool wifiSuccess = wifiModule.initialize();
  
  // Setup and start web server
  webManager.setupRoutes();
  server.begin();
  Serial.println("\nHTTP server started on port " + String(SystemConfig::WEB_SERVER_PORT));
  
  // Print status information
  if (wifiSuccess) {
    SystemUtils::printReadyMessage(wifiModule.getIP());
  } else {
    Serial.println("WARNING: WiFi not connected. Analysis unavailable.");
  }
  
  // Print UART status
  uartController.printStatus();
  
  // Final ready indication
  SystemUtils::blinkSuccess(3);
  Serial.println("System ready for honey badger detection with UART control!");
  Serial.printf("Final free heap: %d bytes\n", ESP.getFreeHeap());
}

void loop() {
  // Handle HTTP requests
  webManager.handleClient();
  
  // Check UART commands (NEW)
  uartController.checkForCommands();
  
  // Heartbeat every 5 seconds
  if (millis() - lastHeartbeat > SystemConfig::HEARTBEAT_INTERVAL) {
    SystemUtils::heartbeat();
    SystemUtils::checkMemory();
    lastHeartbeat = millis();
  }
  
  // WiFi reconnection check every 2 minutes
  if (millis() - lastWifiCheck > SystemConfig::WIFI_RECONNECT_INTERVAL) {
    wifiModule.checkConnection();
    lastWifiCheck = millis();
  }
  
  // Yield to prevent watchdog timer issues
  yield();
}