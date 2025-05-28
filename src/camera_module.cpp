// ============================================================================
// camera_module.cpp - Camera functionality implementation
// ============================================================================
#include "camera_module.h"
#include "system_utils.h"

bool CameraModule::initialize() {
  Serial.println("Initializing camera...");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CameraPins::Y2;
  config.pin_d1 = CameraPins::Y3;
  config.pin_d2 = CameraPins::Y4;
  config.pin_d3 = CameraPins::Y5;
  config.pin_d4 = CameraPins::Y6;
  config.pin_d5 = CameraPins::Y7;
  config.pin_d6 = CameraPins::Y8;
  config.pin_d7 = CameraPins::Y9;
  config.pin_xclk = CameraPins::XCLK;
  config.pin_pclk = CameraPins::PCLK;
  config.pin_vsync = CameraPins::VSYNC;
  config.pin_href = CameraPins::HREF;
  config.pin_sccb_sda = CameraPins::SIOD;
  config.pin_sccb_scl = CameraPins::SIOC;
  config.pin_pwdn = CameraPins::PWDN;
  config.pin_reset = CameraPins::RESET;
  config.xclk_freq_hz = CameraConfig::XCLK_FREQ;
  config.pixel_format = CameraConfig::PIXEL_FORMAT;
  
  // Memory-optimized settings for 4MB PSRAM
  config.frame_size = CameraConfig::FRAME_SIZE;
  config.jpeg_quality = CameraConfig::JPEG_QUALITY;
  config.fb_count = 1;                   // Single frame buffer saves ~40KB RAM
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    SystemUtils::blinkError(5);
    return false;
  }
  
  optimizeSensorSettings();
  initialized = true;
  Serial.println("Camera initialized successfully");
  SystemUtils::blinkSuccess(1);
  return true;
}

void CameraModule::optimizeSensorSettings() {
  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    // Image quality settings
    s->set_quality(s, CameraConfig::JPEG_QUALITY);    // JPEG quality (0-63, lower = better)
    s->set_brightness(s, 0);         // -2 to 2
    s->set_contrast(s, 0);           // -2 to 2  
    s->set_saturation(s, 0);         // -2 to 2
    s->set_sharpness(s, 1);          // -2 to 2 (slight sharpening helps detection)
    s->set_denoise(s, 1);            // Enable noise reduction
    
    // Auto-exposure and gain
    s->set_gainceiling(s, GAINCEILING_2X);
    s->set_exposure_ctrl(s, 1);      // Enable auto exposure
    s->set_aec2(s, 1);               // Enable AEC algorithm
    s->set_gain_ctrl(s, 1);          // Enable auto gain
    
    // White balance
    s->set_whitebal(s, 1);           // Enable auto white balance
    s->set_awb_gain(s, 1);           // Enable AWB gain
    s->set_wb_mode(s, 0);            // Auto WB mode
    
    // Special effects
    s->set_special_effect(s, 0);     // No special effects
    s->set_vflip(s, 0);              // No vertical flip
    s->set_hmirror(s, 0);            // No horizontal mirror
    
    Serial.println("Camera sensor optimized for detection");
  }
}

camera_fb_t* CameraModule::captureImage() {
  if (!initialized) {
    Serial.println("Camera not initialized!");
    return nullptr;
  }
  
  flashOn();
  delay(CameraConfig::FLASH_DURATION);
  camera_fb_t* fb = esp_camera_fb_get();
  flashOff();
  
  if (!fb) {
    Serial.println("Camera capture failed");
    return nullptr;
  }
  
  Serial.printf("Image captured: %dx%d (%u bytes)\n", fb->width, fb->height, fb->len);
  return fb;
}

void CameraModule::releaseFrameBuffer(camera_fb_t* fb) {
  if (fb) {
    esp_camera_fb_return(fb);
  }
}

void CameraModule::flashOn() {
  digitalWrite(SystemPins::FLASH, HIGH);
}

void CameraModule::flashOff() {
  digitalWrite(SystemPins::FLASH, LOW);
}

void CameraModule::printCameraInfo() {
  if (!initialized) {
    Serial.println("Camera not initialized");
    return;
  }
  
  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    Serial.printf("Camera sensor ID: 0x%02X\n", s->id.PID);
    Serial.printf("Frame size: %s\n", 
      CameraConfig::FRAME_SIZE == FRAMESIZE_QVGA ? "QVGA (320x240)" : "Other");
    Serial.printf("JPEG quality: %d\n", CameraConfig::JPEG_QUALITY);
  }
}