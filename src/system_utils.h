// ============================================================================
// system_utils.h - Utility function declarations
// ============================================================================
#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <Arduino.h>
#include "config.h"

namespace SystemUtils {
  // Pin initialization
  void initializePins();
  
  // LED control functions
  void blinkError(int count);
  void blinkSuccess(int count);
  void toggleLED();
  void heartbeat();
  
  // System monitoring
  void checkMemory();
  void printSystemInfo();
  void printWiFiTroubleshooting();
  
  // Startup messages
  void printHeader();
  void printReadyMessage(const String& ip);
}

#endif