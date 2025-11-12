#include "DualSensorMouse.h"
#include <Arduino.h>
#include <Wire.h>
#include <Mouse.h>

// --- 常數設定 ---
const uint8_t SENSOR_X_ADDR = 0x30;
const uint8_t SENSOR_Y_ADDR = 0x31;

// 卡爾曼濾波器參數 (R: 測量雜訊, Q: 處理雜訊)
const float KF_R = 50.0;
const float KF_Q = 1.0;

// 滑鼠控制參數
const int MOVE_THRESHOLD = 5;      // 距離變化超過多少mm才移動
const int MAX_MOVE_SPEED = 30;     // 限制滑鼠單次最大移動量
const int DISTANCE_MAX_LIMIT = 4000; // 忽略超過此距離的讀數 (mm)

// 避免回彈的參數
const int REBOUND_NEAR_LIMIT = 900; // 近距離閾值
const int REBOUND_FAR_LIMIT = 950;  // 遠距離閾值

DualSensorMouse::DualSensorMouse(int xshut_pin_x, int xshut_pin_y)
        : xshutPinX(xshut_pin_x),
        xshutPinY(xshut_pin_y),
        kfX(KF_R, KF_Q, 100.0, 0.0),
        kfY(KF_R, KF_Q, 100.0, 0.0),
        lastDistX(0.0),
        lastDistY(0.0) {
}

bool DualSensorMouse::begin() {
    pinMode(xshutPinX, OUTPUT);
    pinMode(xshutPinY, OUTPUT);

    // 關閉所有感測器
    digitalWrite(xshutPinX, LOW);
    digitalWrite(xshutPinY, LOW);
    delay(10);

    // 依序啟動並設定位址
    if (!initSensor(sensorX, SENSOR_X_ADDR, xshutPinX, "SensorX")) return false;
    if (!initSensor(sensorY, SENSOR_Y_ADDR, xshutPinY, "SensorY")) return false;

    // 初始化滑鼠
    Mouse.begin();

    // 測距一次作為濾波器初始值
    VL53L0X_RangingMeasurementData_t measure;
    sensorX.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) kfX.setInitialValue(measure.RangeMilliMeter);
    lastDistX = kfX.update(measure.RangeMilliMeter);

    sensorY.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) kfY.setInitialValue(measure.RangeMilliMeter);
    lastDistY = kfY.update(measure.RangeMilliMeter);

    Serial.println("✅   雙感測器滑鼠初始化完成！");
    return true;
}

bool DualSensorMouse::initSensor(Adafruit_VL53L0X &sensor, uint8_t addr, int shut_pin, const char* sensorName) {
    digitalWrite(shut_pin, HIGH);
    delay(10);
    if (!sensor.begin(addr)) {
        Serial.print(F("❌ "));
        Serial.print(sensorName);
        Serial.println(F(" 初始化失敗！"));
        return false;
    }

    // --- 設定為長距離模式以獲得更好的效能 ---
    sensor.setSignalRateLimit(0.1);
    sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
    sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
    sensor.setMeasurementTimingBudget(33000);

    Serial.print(F("✅ "));
    Serial.print(sensorName);
    Serial.print(F(" 初始化成功，位址: 0x"));
    Serial.println(addr, HEX);
    return true;
}

void DualSensorMouse::update() {
    VL53L0X_RangingMeasurementData_t measureX, measureY;
    int moveX = 0;
    int moveY = 0;

    // --- 處理 X 軸 ---
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

  // --- 處理 Y 軸 ---
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

    // --- 執行滑鼠移動 ---
    if (moveX != 0 || moveY != 0) {
        Mouse.move(-moveX, moveY, 0);
    }

    delay(100);
}