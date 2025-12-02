#include "DualSensorMouse.h"
#include <Arduino.h>
#include <Wire.h>
#include <Mouse.h>
#include <Keyboard.h>

// --- 常數設定 ---
const uint8_t SENSOR_X_ADDR = 0x30;
const uint8_t SENSOR_Y_ADDR = 0x31;

// 卡爾曼濾波器參數 (R: 測量雜訊, Q: 處理雜訊)
const float KF_R = 50.0;
const float KF_Q = 1.0;

// 滑鼠控制參數
const int MOVE_THRESHOLD = 5;           // 距離變化超過多少mm才移動
const int MAX_MOVE_SPEED = 30;          // 限制滑鼠單次最大移動量
const int DISTANCE_MAX_LIMIT = 4000;    // 忽略超過此距離的讀數 (mm)

// 避免回彈的參數
const int REBOUND_NEAR_LIMIT = 900;     // 近距離閾值
const int REBOUND_FAR_LIMIT = 950;      // 遠距離閾值

// --- 鍵盤模式手勢偵測參數 ---
const int GESTURE_THRESHOLD = 80;           // Y軸距離變化閾值 (mm)，觸發手勢
const unsigned long GESTURE_TIME_WINDOW = 300;  // 手勢時間窗口 (ms)
const unsigned long KEY_DEBOUNCE_TIME = 500;    // 按鍵防抖時間 (ms)

DualSensorMouse::DualSensorMouse(int xshut_pin_x, int xshut_pin_y)
    : xshutPinX(xshut_pin_x),
      xshutPinY(xshut_pin_y),
      kfX(KF_R, KF_Q, 100.0, 0.0),
      kfY(KF_R, KF_Q, 100.0, 0.0),
      lastDistX(0.0),
      lastDistY(0.0),
      currentMode(MODE_MOUSE),
      gestureStartDistY(0.0),
      gestureStartTime(0),
      gestureInProgress(false),
      lastKeyPressTime(0) {
}

bool DualSensorMouse::begin() {
    pinMode(xshutPinX, OUTPUT);
    pinMode(xshutPinY, OUTPUT);

    // 關閉所有感測器
    digitalWrite(xshutPinX, LOW);
    digitalWrite(xshutPinY, LOW);
    delay(10);

    // 依序啟動並設定位址
    if (!initSensor(sensorX, SENSOR_X_ADDR, xshutPinX, "SensorX")) {
        return false;
    }

    if (!initSensor(sensorY, SENSOR_Y_ADDR, xshutPinY, "SensorY")) {
        return false;
    }

    // 初始化滑鼠和鍵盤
    Mouse.begin();
    Keyboard.begin();

    // 測距一次作為濾波器初始值
    VL53L0X_RangingMeasurementData_t measure;
    sensorX.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
        kfX.setInitialValue(measure.RangeMilliMeter);
    }
    lastDistX = kfX.update(measure.RangeMilliMeter);

    sensorY.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
        kfY.setInitialValue(measure.RangeMilliMeter);
    }
    lastDistY = kfY.update(measure.RangeMilliMeter);

    Serial.println("✅   雙感測器滑鼠初始化完成！");
    Serial.println("📌   預設模式: Mouse Mode");
    return true;
}

bool DualSensorMouse::initSensor(Adafruit_VL53L0X &sensor,
                                uint8_t addr,
                                int shut_pin,
                                const char* sensorName) {
    digitalWrite(shut_pin, HIGH);
    delay(10);
    if (!sensor.begin(addr)) {
        Serial.print(F("❌ "));
        Serial.print(sensorName);
        Serial.println(F(" 初始化失敗！"));
        return false;
    }

    // 設定為長距離模式以獲得更好的效能
    sensor.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);

    Serial.print(F("✅ "));
    Serial.print(sensorName);
    Serial.print(F(" 初始化成功，位址: 0x"));
    Serial.println(addr, HEX);
    return true;
}

void DualSensorMouse::setMode(OperationMode mode) {
    if (currentMode != mode) {
        currentMode = mode;
        // 重置手勢偵測狀態
        gestureInProgress = false;
        gestureStartTime = 0;
        
        if (mode == MODE_MOUSE) {
            Serial.println("🖱️   切換到 Mouse Mode");
        } else {
            Serial.println("⌨️   切換到 Keyboard Mode");
        }
    }
}

OperationMode DualSensorMouse::getMode() const {
    return currentMode;
}

void DualSensorMouse::update() {
    // 根據當前模式執行對應的更新邏輯
    if (currentMode == MODE_MOUSE) {
        updateMouseMode();
    } else {
        updateKeyboardMode();
    }
}

void DualSensorMouse::updateMouseMode() {
    VL53L0X_RangingMeasurementData_t measureX, measureY;
    int moveX = 0;
    int moveY = 0;

    // 處理 X 軸
    sensorX.rangingTest(&measureX, false);
    if (measureX.RangeStatus != 4 && measureX.RangeMilliMeter < DISTANCE_MAX_LIMIT) {
        float rawX = measureX.RangeMilliMeter;

        // 避免回彈邏輯：如果距離從很遠突然變得很近，重設濾波器狀態
        if (lastDistX > REBOUND_FAR_LIMIT && rawX < REBOUND_NEAR_LIMIT) {
            kfX.setInitialValue(rawX);
        }

        float filteredX = kfX.update(rawX);
        int deltaX = (int)filteredX - (int)lastDistX;

        if (abs(deltaX) >= MOVE_THRESHOLD) {
            moveX = constrain(deltaX, -MAX_MOVE_SPEED, MAX_MOVE_SPEED);
        }
        lastDistX = filteredX;
    }

    // 處理 Y 軸
    sensorY.rangingTest(&measureY, false);
    if (measureY.RangeStatus != 4 && measureY.RangeMilliMeter < DISTANCE_MAX_LIMIT) {
        float rawY = measureY.RangeMilliMeter;

        // 避免回彈邏輯：如果距離從很遠突然變得很近，重設濾波器狀態
        if (lastDistY > REBOUND_FAR_LIMIT && rawY < REBOUND_NEAR_LIMIT) {
            kfY.setInitialValue(rawY);
        }

        float filteredY = kfY.update(rawY);
        int deltaY = (int)filteredY - (int)lastDistY;

        if (abs(deltaY) >= MOVE_THRESHOLD) {
            moveY = constrain(deltaY, -MAX_MOVE_SPEED, MAX_MOVE_SPEED);
        }
        lastDistY = filteredY;
    }

    // 執行滑鼠移動
    if (moveX != 0 || moveY != 0) {
        Mouse.move(-moveX, moveY, 0);
    }

    delay(100);
}

void DualSensorMouse::updateKeyboardMode() {
    // 使用非阻塞邏輯 (millis) 進行手勢偵測
    unsigned long currentTime = millis();
    
    VL53L0X_RangingMeasurementData_t measureY;
    sensorY.rangingTest(&measureY, false);
    
    // 只處理有效的讀數
    if (measureY.RangeStatus != 4 && measureY.RangeMilliMeter < DISTANCE_MAX_LIMIT) {
        float rawY = measureY.RangeMilliMeter;
        
        // 避免回彈邏輯
        if (lastDistY > REBOUND_FAR_LIMIT && rawY < REBOUND_NEAR_LIMIT) {
            kfY.setInitialValue(rawY);
        }
        
        float filteredY = kfY.update(rawY);
        
        // 手勢偵測邏輯
        if (!gestureInProgress) {
            // 開始新的手勢偵測
            gestureStartDistY = filteredY;
            gestureStartTime = currentTime;
            gestureInProgress = true;
        } else {
            // 檢查是否在時間窗口內
            unsigned long elapsedTime = currentTime - gestureStartTime;
            
            if (elapsedTime <= GESTURE_TIME_WINDOW) {
                // 計算 Y 軸變化量
                float deltaY = filteredY - gestureStartDistY;
                
                // 檢查是否超過閾值且符合防抖條件
                if (abs(deltaY) >= GESTURE_THRESHOLD && 
                    (currentTime - lastKeyPressTime) >= KEY_DEBOUNCE_TIME) {
                    
                    if (deltaY > 0) {
                        // 距離增加 (手遠離感測器) -> DOWN Arrow
                        Keyboard.press(KEY_DOWN_ARROW);
                        delay(50);
                        Keyboard.release(KEY_DOWN_ARROW);
                        Serial.println("⬇️   DOWN Arrow 按下");
                    } else {
                        // 距離減少 (手靠近感測器) -> UP Arrow
                        Keyboard.press(KEY_UP_ARROW);
                        delay(50);
                        Keyboard.release(KEY_UP_ARROW);
                        Serial.println("⬆️   UP Arrow 按下");
                    }
                    
                    lastKeyPressTime = currentTime;
                    gestureInProgress = false;  // 重置手勢狀態
                }
            } else {
                // 超過時間窗口，重置手勢偵測
                gestureInProgress = false;
            }
        }
        
        lastDistY = filteredY;
    }
    
    // 使用較短的延遲以獲得更靈敏的手勢偵測
    delay(20);
}