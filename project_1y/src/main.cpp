#include <Arduino.h>
#include <Wire.h>
#include "DualSensorMouse.h"

// --- 硬體腳位定義 ---
// 根據您的接線修改這兩個腳位
const int XSHUT_PIN_X = 7; // 控制 X 軸的感測器
const int XSHUT_PIN_Y = 4; // 控制 Y 軸的感測器

// --- ESP32 通訊腳位定義 ---
// 連接到 ESP32 的 GPIO 腳位 (需與 ESP32 的 ARDUINO_SIGNAL_PIN 相連)
// ESP32 GPIO 5 -> Arduino 此腳位
const int ESP32_SIGNAL_PIN = 9;  // 可根據接線修改

// 建立控制器物件
DualSensorMouse myMouse(XSHUT_PIN_X, XSHUT_PIN_Y);

// 上一次讀取的 ESP32 信號狀態
int lastSignalState = HIGH;

void setup() {
    Serial.begin(115200);
    while (!Serial); // 等待序列埠連接
    Wire.begin();

    // 設定 ESP32 信號輸入腳位
    pinMode(ESP32_SIGNAL_PIN, INPUT_PULLUP);  // 使用內部上拉電阻，預設為 HIGH

    // 初始化控制器
    if (!myMouse.begin()) {
        Serial.println("🔴 控制器初始化失敗，程式停止。");
        while (1); // 停止執行
    }

    // 讀取初始信號狀態
    lastSignalState = digitalRead(ESP32_SIGNAL_PIN);
    Serial.print("📡   ESP32 初始信號狀態: ");
    Serial.println(lastSignalState == HIGH ? "HIGH (Mouse Mode)" : "LOW (Keyboard Mode)");
}

void loop() {
    // 讀取來自 ESP32 的信號
    int currentSignalState = digitalRead(ESP32_SIGNAL_PIN);

    // 檢測信號變化並切換模式
    if (currentSignalState != lastSignalState) {
        if (currentSignalState == HIGH) {
            // HIGH = "Yes" 被辨識 -> 切換到滑鼠模式
            myMouse.setMode(MODE_MOUSE);
        } else {
            // LOW = "No" 被辨識 -> 切換到鍵盤模式
            myMouse.setMode(MODE_KEYBOARD);
        }
        lastSignalState = currentSignalState;
    }

    // 不斷更新滑鼠/鍵盤狀態
    myMouse.update();
}