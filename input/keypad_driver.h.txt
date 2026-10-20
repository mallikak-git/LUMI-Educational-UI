#pragma once
#include <Arduino.h>

enum class KeyCode {
    NONE = 0,
    NUM_0, NUM_1, NUM_2, NUM_3, NUM_4,
    NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,
    BTN_HOME,
    BTN_MIC,
    BTN_LEARN,
    BTN_MUSIC,
    BTN_STORY,
    BTN_BACK
};

class KeypadDriver {
public:
    KeypadDriver();
    void begin();
    KeyCode update();

private:
    static constexpr uint8_t ROWS = 4;
    static constexpr uint8_t COLS = 4;

    const uint8_t rowPins[ROWS] = {32, 33, 25, 26};
    const uint8_t colPins[COLS] = {27, 14, 12, 13};

    const KeyCode keyMap[ROWS][COLS] = {
        {KeyCode::NUM_1, KeyCode::NUM_2, KeyCode::NUM_3, KeyCode::BTN_HOME},
        {KeyCode::NUM_4, KeyCode::NUM_5, KeyCode::NUM_6, KeyCode::BTN_MIC},
        {KeyCode::NUM_7, KeyCode::NUM_8, KeyCode::NUM_9, KeyCode::BTN_LEARN},
        {KeyCode::BTN_BACK, KeyCode::NUM_0, KeyCode::BTN_MUSIC, KeyCode::BTN_STORY}
    };

    KeyCode lastStableKey;
    KeyCode lastReading;
    unsigned long lastDebounceTime;
    const unsigned long debounceDelay = 35;
};