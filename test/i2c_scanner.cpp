#include <Arduino.h>
#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>

// I2C address of PCF8574 (Default: 0x20, 0x21, etc.)
#define I2C_ADDR 0x20 

// Keypad configuration (4x3)
const byte ROWS = 4;
const byte COLS = 3;

// Define Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Row and Column pin mapping to PCF8574
byte rowPins[ROWS] = {0, 1, 2, 3};  // P0-P3
byte colPins[COLS] = {4, 5, 6};      // P4-P6

// Initialize Keypad
Keypad_I2C keypad = Keypad_I2C(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2C_ADDR);

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);  // SDA = 21, SCL = 22 (ESP32 default I2C pins)
    keypad.begin(makeKeymap(keys));
    Serial.println("Keypad I2C Test Ready...");
}

void loop() {
    char key = keypad.getKey();
    
    if (key) {
        Serial.print("Key Pressed: ");
        Serial.println(key);
    }
}
