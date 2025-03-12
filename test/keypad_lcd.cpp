#include <Arduino.h>
#include <Wire.h>
#include "I2CKeyPad.h"
#include <LiquidCrystal_I2C.h>

const uint8_t KEYPAD_ADDRESS = 0x20;
const uint8_t LCD_ADDRESS = 0x27;  // Common I2C address for LCD, might need adjustment

I2CKeyPad keyPad(KEYPAD_ADDRESS);
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2); // 16 chars, 2 rows

uint32_t lastKeyPressed = 0;
const char KEYPAD_MAPPING[] = "123A456B789C*0#DNF";

void setup() {
    Serial.begin(115200);
    Serial.println("Starting I2C Keypad...");

    Wire.begin(21, 22);  // ESP32 I2C pins
    Wire.setClock(400000);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Feeding Machine");
    lcd.setCursor(0, 1);
    lcd.print("Ready!");

    if (!keyPad.begin()) {
        Serial.println("\nERROR: Cannot communicate with keypad.\nPlease reboot.");
        lcd.clear();
        lcd.print("Keypad Error!");
        while (1);
    }
    Serial.println("I2C Keypad Ready!");
}

void loop() {
    uint32_t now = millis();

    if (now - lastKeyPressed >= 250) {
        lastKeyPressed = now;

        bool connected = keyPad.isConnected();
        bool pressed = keyPad.isPressed();
        uint8_t index = keyPad.getKey();

        if (pressed && index < sizeof(KEYPAD_MAPPING)) {
            Serial.print("✅ Key Pressed: [ ");
            Serial.print(KEYPAD_MAPPING[index]);
            Serial.println(" ]");
            
            // Display pressed key on LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Key Pressed:");
            lcd.setCursor(0, 1);
            lcd.print(KEYPAD_MAPPING[index]);
        }
    }
}
