#include <Arduino.h>
#include <Wire.h>
#include "DualSensorMouse.h"

// --- 硬體腳位定義 ---
// 根據您的接線修改這兩個腳位
const int XSHUT_PIN_X = 7; // 控制 X 軸的感測器
const int XSHUT_PIN_Y = 4; // 控制 Y 軸的感測器

// 建立控制器物件
DualSensorMouse myMouse(XSHUT_PIN_X, XSHUT_PIN_Y);

void setup() {
    Serial.begin(115200);
    while (!Serial); // 等待序列埠連接
    Wire.begin();

    // 初始化控制器
    if (!myMouse.begin()) {
        Serial.println("🔴 控制器初始化失敗，程式停止。");
        while (1); // 停止執行
    }
}

void loop() {
    // 不斷更新滑鼠狀態
    myMouse.update();
}