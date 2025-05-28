// ============================================================================
// wifi_module.h - WiFi functionality declarations
// ============================================================================
#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <WiFi.h>
#include "config.h"

class WiFiModule {
private:
  bool connected = false;
  
public:
  // Initialization and connection
  bool initialize();
  bool reconnect();
  void checkConnection();
  
  // Status checking
  bool isConnected() const;
  wl_status_t getStatus() const { return WiFi.status(); }
  
  // Network information
  String getIP() const;
  int getSignalStrength() const;
  String getGateway() const;
  String getDNS() const;
  
  // Debugging
  void printNetworkInfo();
};

#endif