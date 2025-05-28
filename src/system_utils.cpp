// ============================================================================
// system_utils.cpp - Utility function implementations
// ============================================================================
#include "system_utils.h"

void SystemUtils::initializePins() {
  pinMode(SystemPins::LED, OUTPUT);
  pinMode(SystemPins::FLASH, OUTPUT);
  digitalWrite(SystemPins::LED, LOW);
  digitalWrite(SystemPins::FLASH, LOW);
}

void SystemUtils::blinkError(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(SystemPins::LED, HIGH);
    delay(150);
    digitalWrite(SystemPins::LED, LOW);
    delay(150);
  }
}

void SystemUtils::blinkSuccess(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(SystemPins::LED, HIGH);
    delay(400);
    digitalWrite(SystemPins::LED, LOW);
    delay(400);
  }
}

void SystemUtils::toggleLED() {
  digitalWrite(SystemPins::LED, !digitalRead(SystemPins::LED));
}

void SystemUtils::heartbeat() {
  digitalWrite(SystemPins::LED, HIGH);
  delay(10);
  digitalWrite(SystemPins::LED, LOW);
}

void SystemUtils::checkMemory() {
  static int memoryWarnings = 0;
  if (ESP.getFreeHeap() < SystemConfig::LOW_MEMORY_THRESHOLD) {
    memoryWarnings++;
    if (memoryWarnings % 10 == 1) { // Only log every 10th warning to reduce spam
      Serial.printf("LOW MEMORY WARNING: %d bytes free\n", ESP.getFreeHeap());
    }
  } else {
    memoryWarnings = 0; // Reset warning counter when memory recovers
  }
}

void SystemUtils::printSystemInfo() {
  Serial.printf("Free heap at startup: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM at startup: %d bytes\n", ESP.getFreePsram());
  Serial.printf("Flash size: %d bytes\n", ESP.getFlashChipSize());
}

void SystemUtils::printWiFiTroubleshooting() {
  Serial.println("Make sure your mobile hotspot is:");
  Serial.println("1. Turned on and broadcasting");
  Serial.println("2. Using the correct SSID: " + String(NetworkConfig::SSID));
  Serial.println("3. Using the correct password");
  Serial.println("4. Within range of the ESP32");
}

void SystemUtils::printHeader() {
  Serial.println("\n========================================");
  Serial.println("ESP32-CAM Honey Badger Detector v2.0");
  Serial.println("Refactored & Organized");
  Serial.println("========================================");
}

void SystemUtils::printReadyMessage(const String& ip) {
  Serial.println("Device ready!");
  Serial.println("Web interface: http://" + ip);
  Serial.println("Test capture: http://" + ip + "/capture");
  Serial.println("Status check: http://" + ip + "/status");
  Serial.println("Backend: https://" + String(NetworkConfig::BACKEND_HOST) + ":" + String(NetworkConfig::BACKEND_PORT));
}