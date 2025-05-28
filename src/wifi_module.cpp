// ============================================================================
// wifi_module.cpp - WiFi functionality implementation
// ============================================================================
#include "wifi_module.h"
#include "system_utils.h"

bool WiFiModule::initialize() {
  Serial.println("Connecting to mobile hotspot...");
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(NetworkConfig::SSID, NetworkConfig::PASSWORD);
  
  Serial.print("Connecting to " + String(NetworkConfig::SSID));
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < SystemConfig::WIFI_CONNECT_TIMEOUT) {
    delay(500);
    Serial.print(".");
    SystemUtils::toggleLED();
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    Serial.println();
    Serial.println("WiFi connected successfully!");
    printNetworkInfo();
    SystemUtils::blinkSuccess(2);
    return true;
  } else {
    connected = false;
    Serial.println();
    Serial.println("WiFi connection failed!");
    SystemUtils::printWiFiTroubleshooting();
    SystemUtils::blinkError(3);
    return false;
  }
}

bool WiFiModule::reconnect() {
  Serial.println("WiFi connection lost. Attempting reconnection...");
  
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(NetworkConfig::SSID, NetworkConfig::PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  
  connected = (WiFi.status() == WL_CONNECTED);
  
  if (connected) {
    Serial.println("WiFi reconnected successfully!");
    Serial.println("IP address: " + getIP());
  } else {
    Serial.println("WiFi reconnection failed.");
  }
  
  return connected;
}

void WiFiModule::checkConnection() {
  if (connected && WiFi.status() != WL_CONNECTED) {
    reconnect();
  }
}

bool WiFiModule::isConnected() const {
  return connected && WiFi.status() == WL_CONNECTED;
}

String WiFiModule::getIP() const {
  return WiFi.localIP().toString();
}

int WiFiModule::getSignalStrength() const {
  return WiFi.RSSI();
}

String WiFiModule::getGateway() const {
  return WiFi.gatewayIP().toString();
}

String WiFiModule::getDNS() const {
  return WiFi.dnsIP().toString();
}

void WiFiModule::printNetworkInfo() {
  Serial.println("IP address: " + getIP());
  Serial.println("Signal strength: " + String(getSignalStrength()) + " dBm");
  Serial.println("Gateway: " + getGateway());
  Serial.println("DNS: " + getDNS());
}