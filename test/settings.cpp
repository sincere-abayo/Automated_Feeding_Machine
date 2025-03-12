#include <Arduino.h>
#include <Wire.h>
#include "I2CKeyPad.h"
#include <LiquidCrystal_I2C.h>

// Motor pins
#define IN1 12
#define IN2 14
#define ENA 13
#define IN3 26
#define IN4 27
#define ENB 25

const int motorSpeed = 150;
const int feedingTime = 10000;      // 10 seconds fixed for feeding
const int retractTime = 8000;       // 8 seconds fixed for retracting

// Adjustable times (in seconds)
int eatingTime = 30;    // Default 30 seconds
int shiftDelay = 120;   // Default 120 seconds (2 minutes)

const uint8_t KEYPAD_ADDRESS = 0x20;
const uint8_t LCD_ADDRESS = 0x27;

I2CKeyPad keyPad(KEYPAD_ADDRESS);
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

uint32_t lastKeyPressed = 0;
const char KEYPAD_MAPPING[] = "123A456B789C*0#DNF";  // 12 keys total for 4x3 keypad


String inputBuffer = "";
bool settingMode = false;
int menuState = 0; // 0: main, 1: eating time, 2: shift delay

bool feedingActive = false;

void setupMotor() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENB, OUTPUT);
}

void displayMainMenu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1:Eating Time");
    lcd.setCursor(0, 1);
    lcd.print("2:Shift Delay");
}

void runFeedingCycle() {
    // Forward
    lcd.clear();
    lcd.print("Feeding...");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
    delay(feedingTime);

    // Eating time
    lcd.clear();
    lcd.print("Eating time:");
    lcd.setCursor(0, 1);
    lcd.print(eatingTime);
    lcd.print(" seconds");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
    delay(eatingTime * 1000);

    // Backward
    lcd.clear();
    lcd.print("Retracting...");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
    delay(retractTime);

    // Shift delay
    lcd.clear();
    lcd.print("Next cycle in:");
    lcd.setCursor(0, 1);
    lcd.print(shiftDelay);
    lcd.print(" seconds");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
    delay(shiftDelay * 1000);
}

void handleKeyPress(char key) {
    if (settingMode) {
        if (key >= '0' && key <= '9') {
            inputBuffer += key;
            lcd.setCursor(0, 1);
            lcd.print(inputBuffer);
            lcd.print(" sec    ");
        } else if (key == '#') {
            int value = inputBuffer.toInt();
            if (menuState == 1 && value > 0) {
                eatingTime = value;
            } else if (menuState == 2 && value > 0) {
                shiftDelay = value;
            }
            settingMode = false;
            inputBuffer = "";
            displayMainMenu();
        }
    } else {
        if (key == '1') {
            menuState = 1;
            settingMode = true;
            lcd.clear();
            lcd.print("Set eating time:");
            lcd.setCursor(0, 1);
            inputBuffer = "";
        } else if (key == '2') {
            menuState = 2;
            settingMode = true;
            lcd.clear();
            lcd.print("Set shift delay:");
            lcd.setCursor(0, 1);
            inputBuffer = "";
        } else if (key == '*') {
            feedingActive = !feedingActive;  // Toggle feeding on/off
            if (feedingActive) {
                lcd.clear();
                lcd.print("Feeding Started");
                delay(1000);
            } else {
                lcd.clear();
                lcd.print("Feeding Stopped");
                delay(1000);
            }
            displayMainMenu();
        }
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    Wire.setClock(400000);

    setupMotor();
    
    lcd.init();
    lcd.backlight();
    lcd.print("Feeding Machine");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    delay(2000);

    if (!keyPad.begin()) {
        lcd.clear();
        lcd.print("Keypad Error!");
        while (1);
    }

    displayMainMenu();
}

void loop() {
    uint32_t now = millis();

    if (now - lastKeyPressed >= 250) {
        lastKeyPressed = now;

        if (keyPad.isPressed()) {
            uint8_t index = keyPad.getKey();
            if (index < sizeof(KEYPAD_MAPPING)) {
                handleKeyPress(KEYPAD_MAPPING[index]);
            }
        }
    }
     // Automatic feeding cycle
    if (feedingActive) {
        runFeedingCycle();
    }
}
