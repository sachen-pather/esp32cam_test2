// ============================================================================
// uart_controller.cpp - UART communication implementation (FIXED)
// ============================================================================
#include "uart_controller.h"

bool UARTController::initialize() {
  Serial.println("Initializing UART controller...");
  
  // Initialize UART2 with specified pins
  serial = &Serial2;
  serial->begin(UARTConfig::BAUD_RATE, SERIAL_8N1, SystemPins::UART_RX, SystemPins::UART_TX);
  
  // Clear any existing data
  while (serial->available()) {
    serial->read();
  }
  
  initialized = true;
  deviceConnected = false;  // Initially no device connected
  lastResponseTime = 0;     // No responses yet
  
  Serial.printf("UART initialized - TX: Pin %d, RX: Pin %d, Baud: %d\n", 
                SystemPins::UART_TX, SystemPins::UART_RX, UARTConfig::BAUD_RATE);
  
  // Send initialization message to Arduino
  sendResponse("ESP32_READY");
  
  return true;
}

void UARTController::checkForCommands() {
  if (!initialized || !serial->available()) return;
  
  // Read incoming data
  while (serial->available()) {
    char c = serial->read();
    
    if (c == '\n' || c == '\r') {
      // End of command
      if (inputBuffer.length() > 0) {
        processCommand(inputBuffer);
        inputBuffer = "";
      }
    } else if (c >= ' ' && c <= '~') {
      // Printable character
      inputBuffer += c;
      
      // Prevent buffer overflow
      if (inputBuffer.length() > 32) {
        inputBuffer = "";
      }
    }
  }
}

void UARTController::processCommand(const String& command) {
  String cmd = command;  // Create a non-const copy
  cmd.trim();            // Now we can trim the copy
  Serial.println("UART received: '" + cmd + "'");
  
  // Update response time whenever we receive ANY valid response
  lastResponseTime = millis();
  deviceConnected = true;
  
  if (cmd == "0") {
    lastCommand = 0;
    lastCommandTime = millis();
    Serial.println("Arduino command: 0 (OFF)");
    sendResponse("OK_0");
    
  } else if (cmd == "1") {
    lastCommand = 1;
    lastCommandTime = millis();
    Serial.println("Arduino command: 1 (ON)");
    sendResponse("OK_1");
    
  } else if (cmd == "STATUS") {
    sendStatus();
    
  } else if (cmd == "PING") {
    sendResponse("PONG");
    Serial.println("Arduino ping received - device connected");
    
  } else {
    Serial.println("Unknown UART command: " + cmd);
    sendResponse("ERROR_UNKNOWN");
  }
}

void UARTController::sendResponse(const String& message) {
  if (!initialized) return;
  
  serial->println(message);
  Serial.println("UART sent: " + message);
}

void UARTController::sendStatus() {
  if (!initialized) return;
  
  String status = "STATUS:";
  status += "LAST_CMD=" + String(lastCommand) + ",";
  status += "TIME=" + String(millis()) + ",";
  status += "HEAP=" + String(ESP.getFreeHeap());
  
  sendResponse(status);
}

bool UARTController::hasNewCommand(unsigned long since) const {
  return (lastCommandTime > since && lastCommand != -1);
}

bool UARTController::isDeviceConnected() const {
  if (!initialized) return false;
  
  // Consider device connected if we received a response within the last 30 seconds
  return (millis() - lastResponseTime < 30000);
}

void UARTController::pingDevice() {
  if (!initialized) return;
  
  sendResponse("ESP32_PING");
  Serial.println("Ping sent to Arduino to test connection");
}

void UARTController::printStatus() {
  if (!initialized) {
    Serial.println("UART Controller not initialized");
    return;
  }
  
  Serial.println("=== UART Controller Status ===");
  Serial.printf("TX Pin: %d, RX Pin: %d\n", SystemPins::UART_TX, SystemPins::UART_RX);
  Serial.printf("Baud Rate: %d\n", UARTConfig::BAUD_RATE);
  Serial.printf("Device Connected: %s\n", deviceConnected ? "Yes" : "No");
  Serial.printf("Last Command: %d\n", lastCommand);
  Serial.printf("Last Command Time: %lu ms ago\n", millis() - lastCommandTime);
  Serial.printf("Last Response Time: %lu ms ago\n", millis() - lastResponseTime);
  Serial.printf("Available bytes: %d\n", serial->available());
  Serial.println("==============================");
}

void UARTController::sendTestMessage() {
  if (!initialized) return;
  
  pingDevice();  // Send ping instead of generic test message
}