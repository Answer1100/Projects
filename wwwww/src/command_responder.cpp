/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "command_responder.h"
#include <Arduino.h>
#include <FastLED.h>

// --- LED 設定 ---
#define DATA_PIN 48     // ESP32-S3 開發板內建的 RGB LED 在 GPIO 48
#define NUM_LEDS 1      // 只有 1 顆 LED
#define BRIGHTNESS 20   // 亮度 (0-255)

// 建立 FastLED 的 LED 陣列
CRGB leds[NUM_LEDS];
bool led_initialized = false;

// The default implementation writes out the name of the recognized command
// to the error console. Real applications will want to take some custom
// action instead, and should implement their own versions of this function.
void RespondToCommand(tflite::ErrorReporter* error_reporter,
                      int32_t current_time, const char* found_command,
                      uint8_t score, bool is_new_command) {
  // 第一次呼叫時初始化 LED
  if (!led_initialized) {
    // 初始化 FastLED，WS2812 的晶片組是 NEOPIXEL
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    led_initialized = true;
  }

  if (is_new_command) {
    TF_LITE_REPORT_ERROR(error_reporter, "Heard %s (%d) @%dms", found_command,
                         score, current_time);
    if (strcmp(found_command, "yes") == 0) {
      leds[0] = CRGB::Blue; // 設定為藍色
    } else if (strcmp(found_command, "no") == 0) {
      leds[0] = CRGB::Green; // 設定為綠色
    }
    FastLED.show(); // 點亮 LED
    delay(1000);    // 持續 1 秒
    FastLED.clear(); // 熄滅 LED
    FastLED.show(); // 更新 LED 狀態
  }
}
