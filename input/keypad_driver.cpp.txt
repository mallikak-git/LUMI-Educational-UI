#include "keypad_driver.h"

KeypadDriver::KeypadDriver() 
    : lastStableKey(KeyCode::NONE), lastReading(KeyCode::NONE), lastDebounceTime(0) {}

void KeypadDriver::begin() {
    for (uint8_t r = 0; r < ROWS; ++r) {
        pinMode(rowPins[r], INPUT_PULLUP);
    }
    for (uint8_t c = 0; c < COLS; ++c) {
        pinMode(colPins[c], OUTPUT);
        digitalWrite(colPins[c], HIGH);
    }
}

KeyCode KeypadDriver::update() {
    KeyCode currentReading = KeyCode::NONE;

    for (uint8_t c = 0; c < COLS; ++c) {
        digitalWrite(colPins[c], LOW);
        for (uint8_t r = 0; r < ROWS; ++r) {
            if (digitalRead(rowPins[r]) == LOW) {
                currentReading = keyMap[r][c];
                break;
            }
        }
        digitalWrite(colPins[c], HIGH);
        if (currentReading != KeyCode::NONE) break;
    }

    if (currentReading != lastReading) {
        lastDebounceTime = millis();
    }
    lastReading = currentReading;

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentReading != lastStableKey) {
            lastStableKey = currentReading;
            if (lastStableKey != KeyCode::NONE) {
                return lastStableKey;
            }
        }
    }

    return KeyCode::NONE;
}