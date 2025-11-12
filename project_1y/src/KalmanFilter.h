#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

class KalmanFilter {
public:
    // Constructor
    KalmanFilter(float r, float q, float initial_p, float initial_val);

    // 更新濾波器並返回估計值
    float update(float measurement);

    // 設定初始值
    void setInitialValue(float val);

private:
    float R; // 測量雜訊共變異數
    float Q; // 處理雜訊共變異數
    float P; // 誤差共變異數
    float X; // 估計值 (濾波後的值)
};

#endif // KALMAN_FILTER_H