# ESP32-Based Smart IV Infusion Monitoring & Safety System

Wokwi/PlatformIO educational simulation of an ESP32-based IV infusion monitoring system.

> This is a simulation/prototype, not a clinically validated medical device.

## Features
- Simulated IV fluid-level monitoring using HC-SR04
- Simulated flow/activity detection using PIR
- NORMAL / WARNING / CRITICAL status
- SSD1306 OLED
- Green/yellow/red LEDs
- Buzzer alerts
- Servo-based clamp simulation
- Start/Stop pushbutton
- Serial Monitor output

## GPIO Mapping

| Component | ESP32 GPIO |
|---|---:|
| HC-SR04 TRIG | 5 |
| HC-SR04 ECHO | 19 |
| PIR OUT | 26 |
| OLED SDA | 21 |
| OLED SCL | 22 |
| Servo PWM | 18 |
| Green LED | 25 |
| Yellow LED | 33 |
| Red LED | 32 |
| Buzzer | 23 |
| Pushbutton | 27 |

## Logic

**NORMAL:** distance <= 10 cm and flow detected → green, buzzer off, clamp 0°.

**WARNING:** distance 10–20 cm or no flow → yellow, short beep, clamp 90°.

**CRITICAL:** distance > 20 cm → red, continuous buzzer, clamp 180°.

## Software
Arduino Framework • C/C++ • PlatformIO • Wokwi • Adafruit GFX • Adafruit SSD1306

## Author
**Sri Sujan VM** — Electronics and Communication Engineering (ECE)
