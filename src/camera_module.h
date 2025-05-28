// ============================================================================
// camera_module.h - Camera functionality declarations (FIXED)
// ============================================================================
#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include <Arduino.h>
#include "esp_camera.h"
#include "config.h"

class CameraModule {
private:
  bool initialized;
  
  void optimizeSensorSettings();
  void flashOn();
  void flashOff();
  
public:
  // Constructor
  CameraModule() : initialized(false) {}
  
  // Initialization
  bool initialize();
  bool isInitialized() const { return initialized; }
  
  // Image capture
  camera_fb_t* captureImage();
  void releaseFrameBuffer(camera_fb_t* fb);
  
  // Debugging
  void printCameraInfo();
};

#endif