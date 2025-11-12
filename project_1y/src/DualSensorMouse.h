#ifndef DUAL_SENSOR_MOUSE_H
#define DUAL_SENSOR_MOUSE_H

#include <Adafruit_VL53L0X.h>
#include "KalmanFilter.h"

class DualSensorMouse {
public:
    // Constructor
    DualSensorMouse(int xshut_pin_x, int xshut_pin_y);

    // 初始化感測器和滑鼠
    bool begin();

    // 讀取感測器並更新滑鼠位置
    void update();

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

    // 輔助函式
    bool initSensor(Adafruit_VL53L0X &sensor, uint8_t addr, int shut_pin, const char* sensorName);
};

#endif // DUAL_SENSOR_MOUSE_H