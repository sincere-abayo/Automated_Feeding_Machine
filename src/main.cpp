#include <Arduino.h>
#include <Wire.h>
#include "I2CKeyPad.h"
#include <LiquidCrystal_I2C.h>

// Motor pins
#define IN1 12
#define IN2 14
#define ENA 13
#define IN3 27
#define IN4 26
#define ENB 25

const int forwardSensorPin = 34;  // First sensor for feeding
const int backwardSensorPin = 35; // Second sensor for retracting

const int motorSpeed = 150;
const int feedingTime = 10000;      // 10 seconds fixed for feeding
const int retractTime = 8000;       // 8 seconds fixed for retracting

// Adjustable times (in seconds)
int eatingTime = 10;    // Default 30 seconds
int shiftDelay = 10;   // Default 120 seconds (2 minutes)

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
bool manualMode = false;  // false = automatic, true = manual
bool modeSelected = false;  // Whether the user has selected a mode
bool manualFeedingInProgress = false;  // For manual feeding state tracking

void setupMotor() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENB, OUTPUT);
}

void displayModeSelection() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select Mode:");
    lcd.setCursor(0, 1);
    lcd.print("1:Auto  2:Manual");
}

void displayMainMenu() {
    lcd.clear();
    if (manualMode) {
        lcd.setCursor(0, 0);
        lcd.print("Manual Mode");
        lcd.setCursor(0, 1);
        lcd.print("*:Feed 0:home");
        // delay(2000);
        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("0:Back to Mode");
        // lcd.setCursor(0, 1);
        // lcd.print("Selection");
    } else {
        lcd.setCursor(0, 0);
        lcd.print("1:Eating 2:Shift");
        lcd.setCursor(0, 1);
        lcd.print("*:0:Back");
    }
}

void startFeeding() {
    lcd.clear();
    lcd.print("Feeding...");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
}

void stopMotors() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

void startRetracting() {
    lcd.clear();
    lcd.print("Retracting...");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
}

void runFeedingCycle() {
    // Forward
    startFeeding();
    while(digitalRead(forwardSensorPin) == 1) {
        delay(50); // Small delay for stability
    }
    
    // Eating time
    lcd.clear();
    lcd.print("Eating time:");
    lcd.setCursor(0, 1);
    lcd.print(eatingTime);
    lcd.print(" seconds");
    stopMotors();
    delay(eatingTime * 1000);

    // Backward
    startRetracting();
    while(digitalRead(backwardSensorPin) == 1) {
        delay(50); // Small delay for stability
    }
    stopMotors();

    // Shift delay
    lcd.clear();
    lcd.print("Next cycle in:");
    lcd.setCursor(0, 1);
    lcd.print(shiftDelay);
    lcd.print(" seconds");
    delay(shiftDelay * 1000);
}

void handleManualMode(char key) {
    if (key == '*') {
        if (!manualFeedingInProgress) {
            // Start feeding
            manualFeedingInProgress = true;
            startFeeding();
        }
    } else if (key == '#') {
        // Allow retraction regardless of feeding state
        manualFeedingInProgress = false;
        startRetracting();
        
        // Wait for backward sensor
        while(digitalRead(backwardSensorPin) == 1) {
            delay(50);
        }
        stopMotors();
        lcd.clear();
        lcd.print("Ready for next");
        lcd.setCursor(0, 1);
        lcd.print("feeding");
        delay(2000);
        displayMainMenu();
    } else if (key == '0') {
        // Return to mode selection
        modeSelected = false;
        manualFeedingInProgress = false;
        stopMotors();
        lcd.clear();
        lcd.print("Returning to");
        lcd.setCursor(0, 1);
        lcd.print("mode selection");
        delay(1500);
        displayModeSelection();
    }
}

void handleAutoMode(char key) {
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
        } else if (key == '0') {
            // Return to mode selection
            modeSelected = false;
            feedingActive = false;
            stopMotors();
            lcd.clear();
            lcd.print("Returning to");
            lcd.setCursor(0, 1);
            lcd.print("mode selection");
            delay(1500);
            displayModeSelection();
        }
    }
}

void handleKeyPress(char key) {
    if (!modeSelected) {
        if (key == '1') {
            manualMode = false;
            modeSelected = true;
            lcd.clear();
            lcd.print("Auto Mode Set");
            delay(1500);
            displayMainMenu();
        } else if (key == '2') {
            manualMode = true;
            modeSelected = true;
            lcd.clear();
            lcd.print("Manual Mode Set");
            delay(1500);
            displayMainMenu();
        }
    } else {
        if (manualMode) {
            handleManualMode(key);
        } else {
            handleAutoMode(key);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    Wire.setClock(400000);
    pinMode(forwardSensorPin, INPUT);
    pinMode(backwardSensorPin, INPUT);
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

    displayModeSelection();
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
    
    // Check sensor in manual mode
    if (manualMode && manualFeedingInProgress) {
        if (digitalRead(forwardSensorPin) == 0) {
            // Sensor triggered, stop the motors
            stopMotors();
            lcd.clear();
            lcd.print("Feed position");
            lcd.setCursor(0, 1);
            lcd.print("Press # to retract");
        }
    }
    
    // Automatic feeding cycle
    if (!manualMode && feedingActive) {
        runFeedingCycle();
    }
}
