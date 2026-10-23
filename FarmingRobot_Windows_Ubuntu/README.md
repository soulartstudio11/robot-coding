# Farming Robot — Windows + Ubuntu

## Included
- `arduino/FarmingRobot/FarmingRobot.ino`: Arduino Uno firmware
- `web-terminal/index.html`: browser-based BLE controller/terminal
- `scripts/start-windows.bat`: Windows launcher
- `scripts/start-ubuntu.sh`: Ubuntu launcher

The web UI connects directly to the BLE UART module; no cloud backend or Node.js is required.

## Wiring
Bluetooth TX -> Arduino D10; Bluetooth RX -> Arduino D11; GND -> GND. Confirm the exact module's VCC and RX voltage requirements. Arduino D11 outputs 5 V logic; use a divider/level shifter if the module RX is not 5 V tolerant.

L298N: IN1 D2, IN2 D3, IN3 D4, IN4 D7. Keep ENA/ENB jumpers installed. HC-SR04 TRIG D5, ECHO D6. Moisture analog output A0. Probe servo signal D9. Pump relay/driver signal D8.

Use a separate appropriate supply for motors and pump; join grounds. Never connect a pump directly to an Arduino output. Confirm relay polarity before connecting the pump. First tests: wheels lifted and pump disconnected.

## Arduino setup
1. Open `arduino/FarmingRobot/FarmingRobot.ino` in Arduino IDE.
2. Select Arduino Uno and the correct serial port.
3. Upload. Serial Monitor baud rate is 9600.
4. The firmware expects newline-terminated commands.

Supported commands: `FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `STOP`, `STATUS`, `MOISTURE_CHECK`, `WATER_ON`, `WATER_OFF`, `AUTO_START`, `AUTO_STOP`, `MIN:30`, `MAX:60`.

## Windows
Run `scripts/start-windows.bat`. It starts a local static server and opens `http://localhost:8000`. Python 3 must be installed.

## Ubuntu
From the extracted folder:
```bash
chmod +x scripts/start-ubuntu.sh
./scripts/start-ubuntu.sh
```
Open `http://localhost:8000` in Chrome or a compatible Chromium browser.

## Browser and BLE notes
Web Bluetooth is not supported by every browser or OS. Use a compatible Chrome/Chromium-based browser and a secure context (`localhost` or HTTPS). The page filters for service UUID `FFE0` and uses characteristic `FFE1`.

The terminal can show replies only if the BLE module exposes notification/indication data on that characteristic. If commands are accepted but no reply appears, verify the module's UART-to-BLE forwarding and notification configuration.

## Calibration and limitations
`DRY_RAW`, `WET_RAW`, `SERVO_UP`, and `SERVO_DOWN` are examples and need calibration. The pump output assumes an active-HIGH driver input; adapt for active-LOW relays. Basic auto mode is only a demonstration: it does not map plants or follow a route and may move forward when moisture is above the minimum. Keep an accessible power switch and supervise all tests.
