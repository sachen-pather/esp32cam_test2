// ============================================================================
// config.h - Configuration and constants (WITH UART)
// ============================================================================
#ifndef CONFIG_H
#define CONFIG_H

#include "esp_camera.h"

// Camera pins for AI Thinker ESP32-CAM
namespace CameraPins {
  const int PWDN = 32;
  const int RESET = -1;
  const int XCLK = 0;
  const int SIOD = 26;
  const int SIOC = 27;
  const int Y9 = 35;
  const int Y8 = 34;
  const int Y7 = 39;
  const int Y6 = 36;
  const int Y5 = 21;
  const int Y4 = 19;
  const int Y3 = 18;
  const int Y2 = 5;
  const int VSYNC = 25;
  const int HREF = 23;
  const int PCLK = 22;
}

namespace SystemPins {
  const int LED = 33;
  const int FLASH = 4;
  
  // NEW: UART pins for Arduino communication
  const int UART_TX = 12;  // ESP32 TX → Arduino RX
  const int UART_RX = 13;  // ESP32 RX ← Arduino TX
}

// UART Configuration
namespace UARTConfig {
  const int BAUD_RATE = 115200;     // Standard baud rate
  const int UART_NUM = 2;         // Use UART2 (Serial2)
}

// Use extern for string constants to avoid multiple definitions
namespace NetworkConfig {
  extern const char* SSID;
  extern const char* PASSWORD;
  extern const char* BACKEND_HOST;
  extern const int BACKEND_PORT;
  extern const char* ANALYZE_ENDPOINT;
}

namespace SystemConfig {
  const int SERIAL_BAUD = 115200;
  const int WEB_SERVER_PORT = 80;
  const int WIFI_CONNECT_TIMEOUT = 30;
  const int HTTP_TIMEOUT = 15000;
  const int WIFI_RECONNECT_INTERVAL = 120000;
  const int HEARTBEAT_INTERVAL = 5000;
  const int LOW_MEMORY_THRESHOLD = 25000;
  const int CHUNK_SIZE = 1024;
}

namespace CameraConfig {
  const pixformat_t PIXEL_FORMAT = PIXFORMAT_JPEG;
  const framesize_t FRAME_SIZE = FRAMESIZE_QVGA;    // 320x240
  const int JPEG_QUALITY = 20;              // 20-25 is optimal for detection
  const int XCLK_FREQ = 20000000;
  const int FLASH_DURATION = 50;
}

#endif