#include "KalmanFilter.h"

KalmanFilter::KalmanFilter(float r, float q, float initial_p, float initial_val) {
    R = r;
    Q = q;
    P = initial_p;
    X = initial_val;
}

void KalmanFilter::setInitialValue(float val) {
    X = val;
}

float KalmanFilter::update(float measurement) {
    // 預測步驟
    P = P + Q;

    // 更新步驟
    float K = P / (P + R); // 計算卡爾曼增益
    X = X + K * (measurement - X); // 更新估計值
    P = (1 - K) * P; // 更新誤差共變異數

    return X;
}