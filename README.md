[中文](#中文) | [English](#english)

---

## <a id="中文"></a>
# 具語音關鍵字控制模式的雙感測器懸浮滑鼠 (KWD Dual-Sensor Hover Mouse)

這是一個結合了語音辨識與飛時測距技術的微控制器專案。系統由兩塊主要的開發板協同工作：

1.  **Arduino Leonardo**：負責連接兩個 VL53L0X 雷射測距感測器，讀取手部在空中的距離變化，並將其轉換為滑鼠的移動指令。
2.  **ESP32**：負責執行語音指令辨識模型。當偵測到特定指令（"yes", "no"）時，透過序列埠（Serial）發送對應指令給 Arduino，以切換滑鼠的操作模式。
    *   **"yes" 指令**：啟用懸浮滑鼠的「移動模式」。
    *   **"no" 指令**：啟用「簡報模式」。

這個專案的目標是實現一個可以透過語音指令（'yes' 啟用移動模式，'no' 啟用簡報模式）在不同操作模式之間切換的懸浮滑鼠，提供更直覺的人機互動體驗。

**專案狀態：開發中**
*   [✅] Arduino 端的懸浮滑鼠功能已完成。
*   [⬜️] Arduino 與 ESP32 之間的通訊協定待整合。

## ✨ 主要功能

*   **懸浮滑鼠**：無需接觸任何表面，透過手在空中的位置來控制滑鼠游標。
*   **雙軸控制**：使用兩個 VL53L0X 感測器分別控制滑鼠的 X 軸和 Y 軸移動。
*   **卡爾曼濾波**：對感測器的原始數據進行濾波，使滑鼠移動更加平滑、穩定。
*   **語音切換模式**：透過 ESP32 偵測特定語音指令（"yes", "no"），來切換滑鼠的不同操作模式（移動、簡報）。
    *   **移動模式**：將手部距離變化轉換為滑鼠游標的 X/Y 軸移動。
    *   **簡報模式**：將手部距離變化轉換為簡報控制指令（例如模擬鍵盤的上下左右方向鍵），以進行翻頁。

## 🛠️ 硬體需求

| 元件 | 數量 | 備註 |
| :--- | :--- | :--- |
| **Arduino Leonardo** | 1 | 必須是具有原生 USB HID 功能的板子 |
| **ESP32 開發板** | 1 | 用於語音辨識 |
| **VL53L0X 飛時測距感測器** | 2 | 用於偵測手部距離 |
| **I2S 麥克風模組** | 1 | INMP441，用於接收語音指令 |
| 麵包板與杜邦線 | 若干 | 用於連接電路 |

## 🔌 電路連接

### 1. Arduino Leonardo 與感測器

由於兩個 VL53L0X 感測器的預設 I2C 位址相同，我們需要使用 `XSHUT` 腳位來分別初始化它們，並設定不同的位址。

| VL53L0X (X軸) | Arduino Leonardo |
| :--- | :--- |
| VCC | 5V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |
| XSHUT | **Pin 4** (可自訂) |

| VL53L0X (Y軸) | Arduino Leonardo |
| :--- | :--- |
| VCC | 5V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |
| XSHUT | **Pin 7** (可自訂) |

### 2. ESP32 與麥克風 (INMP441)

| INMP441 | ESP32 |
| :--- | :--- |
| VCC | 3.3V |
| GND | GND |
| SCK | GPIO 10 |
| WS | GPIO 12 |
| SD | GPIO 11 |
| L/R | GND |

## 📝 注意事項 (Notes)

*   **開發板選擇**：本專案的 Arduino 部分**必須**使用 `Arduino Leonardo`、`Arduino Uno R4` 或其他基於 ATmega32U4 晶片的開發板，因為它們才能模擬原生 USB HID 裝置（滑鼠、鍵盤）。`Arduino Uno R3` 或 `Nano` 將無法正常工作。
*   **通訊鮑率**：請確保 Arduino 和 ESP32 程式碼中的序列埠鮑率設定一致（例如 `115200`），否則兩者將無法通訊。
*   **函式庫安裝**：首次使用 PlatformIO 開啟本專案時，它會自動下載所有必要的函式庫。請耐心等待所有依賴項目（Adafruit(4個)、Mouse 等）安裝完成後，再進行編譯（Build）或上傳（Upload）操作，以避免發生錯誤。

## ⚙️ 軟體與安裝

本專案建議使用 **PlatformIO** 進行開發，它可以方便地管理不同開發板的環境和函式庫依賴。

### 1. 安裝 PlatformIO

請先在您的 VS Code 中安裝 PlatformIO IDE 擴充功能。

### 2. 下載專案

```bash
git clone https://github.com/Answer1100/Projects.git
cd Projects
```

---

## <a id="english"></a>
# KWD Dual-Sensor Hover Mouse with Voice-Controlled Modes

This is a microcontroller project that combines voice recognition and Time-of-Flight (ToF) technology. The system consists of two main development boards working in tandem:

1.  **Arduino Leonardo**: Responsible for connecting two VL53L0X laser-ranging sensors, reading the distance changes of a hand in the air, and converting them into mouse movement commands.
2.  **ESP32**: Responsible for running a voice command recognition model. When specific commands ("yes", "no") are detected, it sends corresponding commands to the Arduino via the serial port to switch the mouse's operating mode.
    *   **"yes" command**: Activates the "Movement Mode" for the hover mouse.
    *   **"no" command**: Activates the "Presentation Mode".

The goal of this project is to create a hover mouse that can switch between different operating modes (e.g., movement mode, presentation mode) via voice commands ('yes' to enable movement mode, 'no' to enable presentation mode), offering a more intuitive human-computer interaction experience.

**Project Status: Under Development**
*   [✅] Arduino-side hover mouse functionality completed.
*   [⬜️] Communication protocol between Arduino and ESP32 to be integrated.

## ✨ Key Features

*   **Hover Mouse**: Controls the mouse cursor by hand position in the air, without physical contact with any surface.
*   **Dual-Axis Control**: Uses two VL53L0X sensors to control the X and Y axis movement of the mouse respectively.
*   **Kalman Filter**: Applies a Kalman filter to raw sensor data for smoother and more stable mouse movement.
*   **Voice-Controlled Mode Switching**: The ESP32 detects specific voice commands ("yes", "no") to switch between different operating modes of the mouse (Movement, Presentation).
    *   **Movement Mode**: Converts hand distance changes into X/Y axis movements of the mouse cursor.
    *   **Presentation Mode**: Converts hand distance changes into presentation control commands (e.g., simulating keyboard arrow keys for navigation).

## 🛠️ Hardware Requirements

| Component | Quantity | Notes |
| :--- | :--- | :--- |
| **Arduino Leonardo** | 1 | Must be a board with native USB HID functionality |
| **ESP32 Development Board** | 1 | For voice recognition |
| **VL53L0X Time-of-Flight Sensor** | 2 | For detecting hand distance |
| **I2S Microphone Module** | 1 | INMP441, for receiving voice commands |
| Breadboard & Jumper Wires | Several | For circuit connections |

## 🔌 Circuit Connections

### 1. Arduino Leonardo and Sensors

Since both VL53L0X sensors have the same default I2C address, we need to use the `XSHUT` pins to initialize them separately and assign different addresses.

| VL53L0X (X-axis) | Arduino Leonardo |
| :--- | :--- |
| VCC | 5V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |
| XSHUT | **Pin 4** (Customizable) |

| VL53L0X (Y-axis) | Arduino Leonardo |
| :--- | :--- |
| VCC | 5V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |
| XSHUT | **Pin 7** (Customizable) |

### 2. ESP32 and Microphone (INMP441)

| INMP441 | ESP32 |
| :--- | :--- |
| VCC | 3.3V |
| GND | GND |
| SCK | GPIO 10 |
| WS | GPIO 12 |
| SD | GPIO 11 |
| L/R | GND |

## 📝 Notes

*   **Development Board Selection**: The Arduino part of this project **must** use an `Arduino Leonardo`, `Arduino Uno R4`, or other development boards based on the ATmega32U4 chip, as they can simulate native USB HID devices (mouse, keyboard). `Arduino Uno R3` or `Nano` will not work correctly.
*   **Baud Rate**: Please ensure that the serial port baud rate settings in both Arduino and ESP32 code are consistent (e.g., `115200`); otherwise, they will not be able to communicate.
*   **Library Installation**: When opening this project with PlatformIO for the first time, it will automatically download all necessary libraries. Please wait patiently for all dependencies (e.g., Adafruit libraries, Mouse library) to finish installing before compiling (Build) or uploading (Upload) to avoid errors.

## ⚙️ Software and Installation

This project recommends using **PlatformIO** for development, as it conveniently manages environments and library dependencies for different development boards.

### 1. Install PlatformIO

Please install the PlatformIO IDE extension in your VS Code.

### 2. Download the Project

```bash
git clone https://github.com/Answer1100/Projects.git
cd Projects
```
