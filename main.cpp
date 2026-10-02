#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define TRIG_PIN 5
#define ECHO_PIN 19
#define PIR_PIN 26
#define OLED_SDA 21
#define OLED_SCL 22
#define SERVO_PIN 18
#define GREEN_LED 25
#define YELLOW_LED 33
#define RED_LED 32
#define BUZZER_PIN 23
#define BUTTON_PIN 27

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int servoChannel = 0;
const int servoFrequency = 50;
const int servoResolution = 16;

bool systemRunning = false;
bool lastButtonState = HIGH;
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 250;
bool previousCritical = false;

void setClampAngle(int angle) {
  int minDuty = 1638;
  int maxDuty = 8192;
  int duty = map(angle, 0, 180, minDuty, maxDuty);
  ledcWrite(servoChannel, duty);
}

void allLEDOff() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.0343 / 2.0;
}

void showDisplay(String status, float distance, bool flowDetected, int clampAngle) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("IV INFUSION MONITOR");
  display.println("-------------------");

  display.print("LEVEL : ");
  if (distance <= 10) display.println("HIGH");
  else if (distance <= 20) display.println("MEDIUM");
  else display.println("LOW");

  display.print("DIST  : ");
  if (distance >= 999) display.println("---");
  else { display.print(distance, 0); display.println(" cm"); }

  display.print("FLOW  : ");
  if (flowDetected) display.println("DETECTED");
  else display.println("NO FLOW");

  display.print("CLAMP : ");
  display.print(clampAngle);
  display.println(" DEG");

  display.print("STAT  : ");
  display.println(status);
  display.display();
}

void showStoppedDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 5);
  display.println("IV INFUSION MONITOR");
  display.println("-------------------");
  display.println();
  display.println("SYSTEM: STOPPED");
  display.println();
  display.println("PRESS BUTTON");
  display.println("TO START");
  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ledcSetup(servoChannel, servoFrequency, servoResolution);
  ledcAttachPin(SERVO_PIN, servoChannel);
  setClampAngle(0);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    Serial.println("OLED ERROR");
  else
    Serial.println("OLED OK");

  allLEDOff();
  digitalWrite(GREEN_LED, HIGH);
  noTone(BUZZER_PIN);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 8);
  display.println("IV INFUSION MONITOR");
  display.setCursor(0, 25);
  display.println("SAFETY SYSTEM");
  display.setCursor(0, 45);
  display.println("PRESS BUTTON");
  display.display();

  Serial.println("================================");
  Serial.println("SMART IV INFUSION MONITOR");
  Serial.println("SYSTEM READY");
  Serial.println("PRESS BUTTON TO START");
  Serial.println("================================");
  delay(2000);
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH &&
      millis() - lastButtonTime > debounceDelay) {
    systemRunning = !systemRunning;
    lastButtonTime = millis();

    if (systemRunning)
      Serial.println("SYSTEM: RUNNING");
    else {
      Serial.println("SYSTEM: STOPPED");
      setClampAngle(0);
      noTone(BUZZER_PIN);
    }
  }

  lastButtonState = buttonState;

  float distance = getDistance();
  bool flowDetected = digitalRead(PIR_PIN);

  if (!systemRunning) {
    allLEDOff();
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER_PIN);
    setClampAngle(0);
    showStoppedDisplay();
    delay(300);
    return;
  }

  String status;
  int clampAngle;

  if (distance > 20 || distance >= 999) {
    status = "CRITICAL";
    clampAngle = 180;
    allLEDOff();
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER_PIN, 2000);
    setClampAngle(clampAngle);

    if (!previousCritical) {
      previousCritical = true;
      Serial.println("!!! CRITICAL ALERT !!!");
      Serial.println("IV FLUID LEVEL VERY LOW");
    }
  }
  else if (distance > 10 || !flowDetected) {
    status = "WARNING";
    clampAngle = 90;
    allLEDOff();
    digitalWrite(YELLOW_LED, HIGH);
    tone(BUZZER_PIN, 1200, 200);
    setClampAngle(clampAngle);
    previousCritical = false;
    Serial.println("WARNING: CHECK INFUSION");
  }
  else {
    status = "NORMAL";
    clampAngle = 0;
    allLEDOff();
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER_PIN);
    setClampAngle(clampAngle);
    previousCritical = false;
    Serial.println("INFUSION STATUS: NORMAL");
  }

  showDisplay(status, distance, flowDetected, clampAngle);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Flow: ");
  if (flowDetected) Serial.print("DETECTED");
  else Serial.print("NO FLOW");
  Serial.print(" | Status: ");
  Serial.print(status);
  Serial.print(" | Clamp: ");
  Serial.print(clampAngle);
  Serial.println(" deg");

  delay(500);
}
