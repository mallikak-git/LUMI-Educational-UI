#include "ui_manager.h"

#define COLOR_BG        0xEFFE // Soft Pastel Blue
#define COLOR_TEXT      0x2124 // Dark Slate
#define COLOR_CARD      0xFFFF // White Surface
#define COLOR_CYAN      0x3D7E // Accent Teal

// Embedded offline library so the device works even before SD card integration
static const EduItem OFFLINE_DB[] = {
    {"Animal", "Elephant", "A giant gentle animal with a long trunk!\nSound: Pawoo!\nFact: Elephants greet friends by wrapping trunks.", "Which animal has a long trunk?", "Elephant", "Rabbit", 1},
    {"Animal", "Lion", "King of the jungle with a big mane!\nSound: Roar!\nFact: Lions live together in groups called prides.", "What sound does a lion make?", "Meow", "Roar", 2},
    {"Fruit", "Apple", "Crisp and juicy sweet fruit!\nColor: Red or Green\nFact: Apples float because 25% of their volume is air.", "What color can an apple be?", "Red", "Blue", 1},
    {"Fruit", "Banana", "Sweet fruit with its own yellow peel wrapper!\nRich in potassium to give you energy to play.", "What color is a ripe banana?", "Purple", "Yellow", 2},
    {"Shape", "Circle", "A perfectly round shape with zero corners!\nExamples: The Sun, a clock, a coin, and a wheel.", "How many corners does a circle have?", "Zero", "Four", 1},
    {"Shape", "Triangle", "A shape with 3 straight sides and 3 pointy corners!\nExamples: A pizza slice and a roof.", "How many sides does a triangle have?", "3", "5", 1},
    {"Number", "Number 5", "High five! You have 5 fingers on each hand\nand 5 toes on each foot to help you balance.", "How many fingers on one hand?", "5", "10", 1},
    {"Alphabet", "Letter A", "A is for Apple! Crisp, sweet, and red.\nSay it out loud: 'Ah-Ah-Apple!'", "What does Letter A stand for?", "Apple", "Boat", 1}
};
static const uint8_t TOTAL_OFFLINE_ITEMS = sizeof(OFFLINE_DB) / sizeof(EduItem);

UIManager::UIManager()
    : currentState(ScreenState::HOME), currentFace(LumiFaceState::IDLE),
      lastAnimUpdate(0), animFrame(0), listeningActive(false),
      currentCorrectOpt(1), currentContentIndex(0) {
    loadOfflineItem(0);
}

void UIManager::begin() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COLOR_BG);
    setScreenState(ScreenState::HOME);
}

void UIManager::loadOfflineItem(uint8_t index) {
    currentContentIndex = index % TOTAL_OFFLINE_ITEMS;
    currentTitle = OFFLINE_DB[currentContentIndex].title;
    currentBody = OFFLINE_DB[currentContentIndex].body;
    quizOptA = OFFLINE_DB[currentContentIndex].optA;
    quizOptB = OFFLINE_DB[currentContentIndex].optB;
    currentCorrectOpt = OFFLINE_DB[currentContentIndex].correctOpt;
}

void UIManager::nextContentItem() {
    loadOfflineItem(currentContentIndex + 1);
    if (currentState == ScreenState::LEARNING || currentState == ScreenState::QUIZ) {
        setScreenState(currentState);
    }
}

void UIManager::setScreenState(ScreenState state) {
    currentState = state;
    tft.fillScreen(COLOR_BG);

    switch (currentState) {
        case ScreenState::HOME:     renderHomeScreen(); break;
        case ScreenState::LEARNING: renderLearningScreen(); break;
        case ScreenState::QUIZ:     renderQuizScreen(); break;
        case ScreenState::STORY:    renderStoryScreen(); break;
        case ScreenState::SETTINGS: renderSettingsScreen(); break;
        case ScreenState::PROGRESS: renderProgressScreen(); break;
    }
}

void UIManager::setLumiFace(LumiFaceState face) {
    currentFace = face;
    if (currentState == ScreenState::HOME) {
        drawLumiFace(160, 95, currentFace, animFrame);
    }
}

void UIManager::setContent(const String& title, const String& body) {
    currentTitle = title;
    currentBody = body;
    if (currentState == ScreenState::LEARNING || currentState == ScreenState::STORY) {
        setScreenState(currentState);
    }
}

void UIManager::setQuiz(const String& question, const String& optA, const String& optB) {
    currentTitle = question;
    quizOptA = optA;
    quizOptB = optB;
    setScreenState(ScreenState::QUIZ);
}

void UIManager::startListening() {
    listeningActive = true;
    setLumiFace(LumiFaceState::LISTENING);
    renderHeader("I'm Listening!");
    tft.fillRoundRect(30, 165, 260, 48, 8, 0xFC72);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(55, 182);
    tft.print("Listening... Speak into the mic!");
}

void UIManager::stopListening() {
    listeningActive = false;
    setLumiFace(LumiFaceState::THINKING);
    renderHeader("Processing...");
    tft.fillRoundRect(30, 165, 260, 48, 8, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.setCursor(70, 182);
    tft.print("Thinking about your answer...");
}

void UIManager::handleInput(KeyCode key) {
    if (key == KeyCode::BTN_MIC) {
        if (!listeningActive) startListening();
        else stopListening();
        return;
    }

    if (key == KeyCode::BTN_HOME) {
        listeningActive = false;
        setScreenState(ScreenState::HOME);
        return;
    }

    // Quiz evaluation
    if (currentState == ScreenState::QUIZ) {
        if (key == KeyCode::NUM_1 || key == KeyCode::NUM_2) {
            uint8_t selected = (key == KeyCode::NUM_1) ? 1 : 2;
            if (selected == currentCorrectOpt) {
                setContent("Quiz Result", "Correct! You got it right! ⭐");
            } else {
                setContent("Quiz Result", "Nice try! Let's review and try again!");
            }
            setScreenState(ScreenState::LEARNING);
            return;
        }
    }

    // Cycle items in Learn mode with digit keys
    if (currentState == ScreenState::LEARNING && key == KeyCode::NUM_0) {
        nextContentItem();
        return;
    }

    // Screen navigation
    switch (key) {
        case KeyCode::BTN_LEARN: setScreenState(ScreenState::LEARNING); break;
        case KeyCode::BTN_STORY: setScreenState(ScreenState::STORY); break;
        case KeyCode::BTN_MUSIC: setScreenState(ScreenState::QUIZ); break;
        case KeyCode::NUM_9:     setScreenState(ScreenState::PROGRESS); break;
        case KeyCode::BTN_BACK:  setScreenState(ScreenState::SETTINGS); break;
        default: break;
    }
}

void UIManager::updateAnimation() {
    if (millis() - lastAnimUpdate > 350) {
        lastAnimUpdate = millis();
        animFrame = (animFrame + 1) % 4;

        if (currentState == ScreenState::HOME) {
            drawLumiFace(160, 95, currentFace, animFrame);
        }
    }
}

void UIManager::renderStatusBar(int batteryPct, bool micActive) {
    tft.drawRoundRect(275, 8, 28, 14, 2, TFT_DARKGREY);
    tft.fillRect(303, 12, 2, 6, TFT_DARKGREY);
    int fillW = map(batteryPct, 0, 100, 0, 24);
    tft.fillRect(277, 10, fillW, 10, TFT_GREEN);

    if (micActive) {
        tft.fillCircle(260, 15, 4, TFT_RED);
    }
}

void UIManager::renderHeader(const char* title) {
    tft.fillRect(0, 0, 320, 32, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    tft.setCursor(15, 8);
    tft.print(title);
    renderStatusBar(95, listeningActive);
    tft.drawFastHLine(0, 32, 320, TFT_LIGHTGREY);
}

void UIManager::drawLumiFace(int x, int y, LumiFaceState state, int frame) {
    tft.fillCircle(x, y, 42, 0xFEE0);
    tft.drawCircle(x, y, 42, TFT_ORANGE);

    if (state == LumiFaceState::LISTENING) {
        tft.fillCircle(x - 14, y - 8, 6, TFT_BLACK);
        tft.fillCircle(x + 14, y - 8, 6, TFT_BLACK);
        tft.fillCircle(x - 12, y - 10, 2, TFT_WHITE);
        tft.fillCircle(x + 16, y - 10, 2, TFT_WHITE);
    } else if (state == LumiFaceState::THINKING) {
        tft.fillCircle(x - 14, y - 12, 5, TFT_BLACK);
        tft.fillCircle(x + 14, y - 12, 5, TFT_BLACK);
    } else {
        if (frame == 3) {
            tft.drawFastHLine(x - 20, y - 8, 12, TFT_BLACK);
            tft.drawFastHLine(x + 8, y - 8, 12, TFT_BLACK);
        } else {
            tft.fillCircle(x - 14, y - 8, 5, TFT_BLACK);
            tft.fillCircle(x + 14, y - 8, 5, TFT_BLACK);
        }
    }

    tft.fillCircle(x - 24, y + 6, 4, 0xFC72);
    tft.fillCircle(x + 24, y + 6, 4, 0xFC72);

    tft.fillRect(x - 15, y + 10, 30, 15, 0xFEE0);
    if (state == LumiFaceState::SPEAKING) {
        int mouthHeight = (frame % 2 == 0) ? 8 : 4;
        tft.fillRoundRect(x - 8, y + 12, 16, mouthHeight, 3, TFT_BLACK);
    } else {
        tft.drawCircle(x, y + 8, 8, TFT_BLACK);
        tft.fillRect(x - 10, y + 6, 20, 6, 0xFEE0);
    }
}

void UIManager::renderHomeScreen() {
    renderHeader("Hi, Aarav! :)");[cite: 6]
    drawLumiFace(160, 95, currentFace, animFrame);

    tft.fillRoundRect(20, 175, 280, 50, 10, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.setCursor(35, 195);
    tft.print("[1] Learn    [2] Story    [3] Quiz    [Mic] Talk");
}

void UIManager::renderLearningScreen() {
    renderHeader("Learn Mode");[cite: 1, 6]
    tft.fillRoundRect(15, 45, 290, 175, 8, COLOR_CARD);
    
    tft.setTextColor(COLOR_CYAN);
    tft.setTextSize(2);
    tft.setCursor(30, 60);
    tft.print(currentTitle);

    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.setCursor(30, 95);
    tft.print(currentBody);

    tft.setCursor(30, 195);
    tft.setTextColor(TFT_DARKGREY);
    tft.print("Press [0] for Next Item | [3] for Quiz");
}

void UIManager::renderQuizScreen() {
    renderHeader("Quiz Challenge");[cite: 1]
    tft.fillRoundRect(15, 45, 290, 180, 8, COLOR_CARD);

    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    tft.setCursor(25, 60);
    const char* q = OFFLINE_DB[currentContentIndex].quizQ;
    tft.print(q ? q : "Choose the answer:");

    tft.fillRoundRect(25, 110, 270, 40, 6, 0x10B981);
    tft.setCursor(35, 122);
    tft.setTextColor(TFT_WHITE);
    tft.print("1. ");
    tft.print(quizOptA);

    tft.fillRoundRect(25, 160, 270, 40, 6, 0xF43F5E);
    tft.setCursor(35, 172);
    tft.print("2. ");
    tft.print(quizOptB);
}

void UIManager::renderStoryScreen() {
    renderHeader("Story Time");[cite: 1, 6]
    tft.fillRoundRect(15, 45, 290, 175, 8, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    tft.setCursor(30, 65);
    tft.print("The Clever Fox 🦊");
    tft.setTextSize(1);
    tft.setCursor(30, 105);
    tft.print("Once upon a time in an enchanted forest,\na curious fox named Rusty found a magic\nglowing compass under an oak tree...");
}

void UIManager::renderSettingsScreen() {
    renderHeader("LUMI Setup");[cite: 1, 9]
    tft.fillRoundRect(15, 45, 290, 175, 8, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.setCursor(30, 65);
    tft.println("Battery: 100% (CR2032 Active)");
    tft.setCursor(30, 85);
    tft.println("Volume: Level 4");
    tft.setCursor(30, 105);
    tft.println("Storage: MicroSD Ready");
}

void UIManager::renderProgressScreen() {
    renderHeader("My Learning Stars");
    tft.fillRoundRect(15, 45, 290, 175, 8, COLOR_CARD);

    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    tft.setCursor(30, 60);
    tft.print("Weekly Progress: 70%");

    tft.drawRoundRect(30, 95, 260, 22, 5, TFT_DARKGREY);
    tft.fillRoundRect(32, 97, 182, 18, 4, 0x3B82F6);

    tft.setTextSize(1);
    tft.setCursor(30, 130);
    tft.print("Quizzes Completed: 8 / 10");
    tft.setCursor(30, 150);
    tft.print("Stories Listened: 5");
    tft.setCursor(30, 175);
    tft.setTextColor(0xD977);
    tft.print("Awesome job, Aarav! 3 Stars! ⭐⭐⭐");
}