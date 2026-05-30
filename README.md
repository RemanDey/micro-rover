# Micro-Rover (Zade) - WiFi Control System

This project transforms an ESP8266-based rover into a fully interactive robot with a web-based dashboard. It features real-time joystick control, trajectory tracking, path drawing, and tilt (accelerometer) control.

## Features
- **Web Dashboard**: Control the rover from any browser (mobile or desktop).
- **Dual Mode WiFi**: Operates as an Access Point (direct connect) and a Station (connects to your home WiFi).
- **Visual Trajectory**: Real-time simulation of the rover's path on a canvas.
- **Draw Mode**: Draw a path on your screen and the rover will follow it.
- **Tilt Control**: Use your smartphone's accelerometer to steer by tilting.
- **mDNS Support**: Access the rover via `http://micro-rover.local`.

---

## 🛠 Hardware Requirements
1. **Microcontroller**: ESP8266 (NodeMCU or Wemos D1 Mini).
2. **Motor Driver**: L298N, L293D, or DRV8833.
3. **Chassis**: 2WD Differential Drive Robot Kit.
4. **Power**: 5V Power Bank and an USB Cable.

### Wiring Diagram
| ESP8266 Pin | Motor Driver Pin | Function |
|-------------|------------------|----------|
| **D1**      | IN1              | Left Motor Forward |
| **D2**      | IN2              | Left Motor Backward |
| **D6**      | IN3              | Right Motor Forward |
| **D7**      | IN4              | Right Motor Backward |
| **D0**      | ENA              | Left Motor Speed (PWM) |
| **D5**      | ENB              | Right Motor Speed (PWM) |
| **GND**     | GND              | Common Ground |

---

## Software Setup

1. **Arduino IDE**: Install the latest version.
2. **ESP8266 Board Package**: 
   - Go to `File > Preferences`.
   - Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to Additional Boards Manager URLs.
   - Go to `Tools > Board > Boards Manager`, search for `esp8266` and install.
3. **Configuration**:
   - Open `main.ino`.
   - Update the `WIFI_STA_SSID` and `WIFI_STA_PASS` with your home WiFi credentials.
4. **Upload**: Select your board (e.g., NodeMCU 1.0) and click **Upload**.

---

## How to Control

### Web Interface
1. **Connect**: TWO WAYS TO CONNECT TO ROVER
   -i  Connect to the Micro-Rover WiFi network `micro_rover` (Password: `12345678`)[this is for the case when you don't want to connect to any wifi network and control the rover directly from the connected device]
   -ii Configure the WIFI_STA_SSID and WIFI_STA_PASS with your home wifi credentials
2. Open your browser and go to `http://micro-rover.local` or the displayed IP in the serial monitor.
3. **Joystick**: Drag the blue circle to move.
4. **Keyboard**: Use `WASD` or Arrow keys.
5. **Draw Mode**: Click "Draw Mode: ON", draw a pattern using the joystick area or Keyboard(drawn trajectory will appear in yellow), and click "Follow Path".

---

## Advanced Control (Python)

The `scripts/` folder contains Python scripts for advanced automation and external control. These scripts allow you to control the rover programmatically from a PC, Raspberry Pi, or any device running Python.

### Usage:
1. Ensure you are on the same network as the rover.
2. Install requirements: `pip install requests`.
3. Run a control script:
   ```bash
   python scripts/keyboard_control.py
   ```

### API Endpoints:
If you wish to write your own scripts, the rover listens to:
- **Endpoint**: `http://<IP_ADDRESS>/move?left=<val>&right=<val>`
- **Values**: `-1023` to `1023`.

---

## Safety Notes
- Ensure the common ground (GND) is connected between the ESP8266 and the motor driver.
- Do not power the motors directly from the ESP8266 3.3V pin; use an external battery.
- Always test with the rover on a stand (wheels off the ground) first!

## License
MIT License. Free to use and modify.
```

<