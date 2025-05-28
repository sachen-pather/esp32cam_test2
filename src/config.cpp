// ============================================================================
// config.cpp - Configuration constants definitions
// ============================================================================
#include "config.h"

// Define the NetworkConfig constants here (only once)
namespace NetworkConfig {
  const char* SSID = "LA";        // Replace with your mobile hotspot name
  const char* PASSWORD = "password"; // Replace with your mobile hotspot password
  const char* BACKEND_HOST = "honey-badger-detection20250524205314.azurewebsites.net";
  const int BACKEND_PORT = 443;
  const char* ANALYZE_ENDPOINT = "/api/Camera/analyze";
}