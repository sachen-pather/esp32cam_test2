// ============================================================================
// uart_controller.h - UART communication with Arduino
// ============================================================================
#ifndef UART_CONTROLLER_H
#define UART_CONTROLLER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "config.h"

class UARTController {
private:
  HardwareSerial* serial;
  bool initialized = false;
  bool deviceConnected = false;  // NEW: Track if Arduino is actually responding
  int lastCommand = -1;
  unsigned long lastCommandTime = 0;
  unsigned long lastResponseTime = 0;  // NEW: Track last response from Arduino
  String inputBuffer = "";
  
  // Private method for processing commands
  void processCommand(const String& command);
  
public:
  // Initialization
  bool initialize();
  bool isInitialized() const { return initialized; }
  bool isDeviceConnected() const;  // NEW: Check if Arduino is actually connected
  
  // Communication
  void checkForCommands();
  void sendResponse(const String& message);
  void sendStatus();
  void pingDevice();  // NEW: Send ping to check if Arduino responds
  
  // Command handling
  int getLastCommand() const { return lastCommand; }
  unsigned long getLastCommandTime() const { return lastCommandTime; }
  unsigned long getLastResponseTime() const { return lastResponseTime; }
  bool hasNewCommand(unsigned long since = 0) const;
  
  // Debugging
  void printStatus();
  void sendTestMessage();
};

#endif