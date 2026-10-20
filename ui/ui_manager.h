#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../input/keypad_driver.h"

enum class ScreenState {
    HOME,
    LEARNING,
    QUIZ,
    STORY,
    SETTINGS,
    PROGRESS
};

enum class LumiFaceState {
    IDLE,
    LISTENING,
    THINKING,
    SPEAKING
};

struct EduItem {
    const char* category;
    const char* title;
    const char* body;
    const char* quizQ;
    const char* optA;
    const char* optB;
    uint8_t correctOpt;
};

class UIManager {
public:
    UIManager();
    void begin();
    void handleInput(KeyCode key);
    void updateAnimation();

    // Mode and Screen controls
    void setScreenState(ScreenState state);
    void setLumiFace(LumiFaceState face);
    void setContent(const String& title, const String& body);
    void setQuiz(const String& question, const String& optA, const String& optB);

    // Audio / Voice state integration
    void startListening();
    void stopListening();
    bool isListening() const { return listeningActive; }

    // Offline content browsing
    void nextContentItem();

private:
    TFT_eSPI tft;
    ScreenState currentState;
    LumiFaceState currentFace;
    unsigned long lastAnimUpdate;
    int animFrame;
    bool listeningActive;

    String currentTitle;
    String currentBody;
    String quizOptA;
    String quizOptB;
    uint8_t currentCorrectOpt;
    uint8_t currentContentIndex;

    // Drawing primitives
    void renderHomeScreen();
    void renderLearningScreen();
    void renderQuizScreen();
    void renderStoryScreen();
    void renderSettingsScreen();
    void renderProgressScreen();
    void renderHeader(const char* title);
    void renderStatusBar(int batteryPct, bool micActive);
    void drawLumiFace(int x, int y, LumiFaceState state, int frame);
    void loadOfflineItem(uint8_t index);
};