#ifndef DUAL_SENSOR_MOUSE_H
#define DUAL_SENSOR_MOUSE_H

#include <Adafruit_VL53L0X.h>
#include "KalmanFilter.h"

// 操作模式列舉
enum OperationMode {
    MODE_MOUSE,     // 滑鼠模式 (由 "yes" 觸發)
    MODE_KEYBOARD   // 鍵盤模式 (由 "no" 觸發)
};

class DualSensorMouse {
public:
    // Constructor
    DualSensorMouse(int xshut_pin_x, int xshut_pin_y);

    // 初始化感測器和滑鼠
    bool begin();

    // 讀取感測器並更新滑鼠/鍵盤狀態
    void update();

    // 設定操作模式 (由外部 ESP32 信號控制)
    void setMode(OperationMode mode);

    // 取得當前操作模式
    OperationMode getMode() const;

private:
    // 感測器物件
    Adafruit_VL53L0X sensorX;
    Adafruit_VL53L0X sensorY;

    // XSHUT 腳位
    int xshutPinX;
    int xshutPinY;

    // 卡爾曼濾波器
    KalmanFilter kfX;
    KalmanFilter kfY;

    // 上一次的距離
    float lastDistX;
    float lastDistY;

    // 當前操作模式
    OperationMode currentMode;

    // --- 手勢偵測相關變數 (用於鍵盤模式) ---
    float gestureStartDistY;        // 手勢開始時的 Y 軸距離
    unsigned long gestureStartTime; // 手勢開始時間 (millis)
    bool gestureInProgress;         // 是否正在偵測手勢
    unsigned long lastKeyPressTime; // 上一次按鍵時間 (避免重複觸發)

    // 輔助函式
    bool initSensor(Adafruit_VL53L0X &sensor, uint8_t addr, int shut_pin, const char* sensorName);

    // 滑鼠模式更新
    void updateMouseMode();

    // 鍵盤模式更新 (手勢偵測)
    void updateKeyboardMode();
};

#endif // DUAL_SENSOR_MOUSE_H