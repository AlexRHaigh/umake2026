#include <Arduino.h>
#include <LiquidCrystal.h>

#define JOY_X 34
#define JOY_Y 32
#define JOY_SW 39

// Thresholds for direction detection
#define DEADZONE_LOW 1500
#define DEADZONE_HIGH 2600

#define RS 22
#define E 5
#define D4 17
#define D5 21
#define D6 19
#define D7 18

// LiquidCrystal Setup
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

int score = 0
int timer = 0

void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  pinMode(JOY_SW, INPUT_PULLUP);

  Serial.println("Joystick Debug Started");

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Timer: 0 Sec");

  lcd.setCursor(0,1);
  lcd.print("Score: 0")
}

void loop() {

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);
  int sw = digitalRead(JOY_SW);

  timer++;

  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.print("Timer: ");
  lcd.print(timer);
  lcd.print(" Sec   ");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.print("Score: ");
  lcd.print(score);
  lcd.print("     ");

  Serial.print("X: ");
  Serial.print(x);
  Serial.print("  Y: ");
  Serial.print(y);

  Serial.print("  Direction: ");

  if (x < DEADZONE_LOW) {
    Serial.print("LEFT");
  }
  else if (x > DEADZONE_HIGH) {
    Serial.print("RIGHT");
  }
  else if (y < DEADZONE_LOW) {
    Serial.print("DOWN");
  }
  else if (y > DEADZONE_HIGH) {
    Serial.print("UP");
  }
  else {
    Serial.print("CENTER");
  }

  Serial.print("  Button: ");

  if (sw == LOW) {
    Serial.print("PRESSED");
  } else {
    Serial.print("RELEASED");
  }

  Serial.println();

  delay(300);
}