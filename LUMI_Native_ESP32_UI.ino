/*
  LUMI — Native ESP32 + ILI9341 UI scaffold
  Target: ESP32 DevKit / ESP32-WROOM-32, ILI9341 240x320 portrait
  Libraries: Adafruit GFX Library, Adafruit ILI9341

  IMPORTANT:
  - Review/compile scaffold only; hardware is not available.
  - TFT pins below are TEMPORARY placeholders, not verified wiring.
  - Keypad scanning is disabled until the complete 4x3 matrix wiring is checked.
  - Do not upload/run until TFT and keypad pins are verified.
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// TEMPORARY TFT PIN PLACEHOLDERS — verify before hardware use.
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// Disabled until all keypad wires and pull-ups are verified.
#define LUMI_ENABLE_KEYPAD 0
const uint8_t ROW_PINS[4] = {14, 27, 16, 12};
// Column pin choices are placeholders. GPIO35 needs an external pull-up.
const uint8_t COL_PINS[3] = {35, 13, 26};

const uint16_t C_BG=0xF5FF, C_NAVY=0x12EB, C_YELLOW=0xFECA;
const uint16_t C_MINT=0x976F, C_CORAL=0xFD35, C_PURPLE=0xC65F;
const uint16_t C_WHITE=ILI9341_WHITE, C_GREEN=0x3666, C_GRAY=0xBDF7;

enum Screen { HOME, LEARN_MENU, NUMBERS, ANIMALS, COLORS, SHAPES, FRUITS, ABC,
              PLAY_MENU, MATCH_GAME, STORIES_MENU, STORY_PAGE, QUIZ_PAGE };
Screen currentScreen=HOME;
uint8_t learnPage=0, numberValue=1, storyIndex=0, quizIndex=0;
uint8_t selectedAnswer=255, score=0, matchIndex=0, letterIndex=0;
bool answerChecked=false, matchRevealed=false;

const char* learnItems[]={"Numbers 1-20","Animals","Colors","Shapes","Fruits","ABC"};
const uint8_t LEARN_COUNT=sizeof(learnItems)/sizeof(learnItems[0]);
const char* stories[]={"Milo and the Moon","A Tiny Seed","The Kind Elephant"};
const char* storyTexts[]={
  "Milo saw the moon.\nIt looked like a\nsilver smile.\nHe waved goodnight.",
  "A seed slept in soil.\nRain came, then sun.\nA green shoot grew\ninto a little plant.",
  "An elephant found\na thirsty bird.\nHe shared water.\nThey became friends."
};
const uint8_t STORY_COUNT=sizeof(stories)/sizeof(stories[0]);
struct QuizItem { const char* question; const char* answers[3]; uint8_t correct; };
const QuizItem quizItems[]={
  {"How many sides\nhas a triangle?",{"2","3","4"},1},
  {"Which one is\nusually yellow?",{"Banana","Grape","Blueberry"},0},
  {"Which animal\nsays 'meow'?",{"Dog","Cow","Cat"},2},
  {"What comes after\nnumber 4?",{"3","5","6"},1},
  {"Which shape is\nround?",{"Circle","Square","Triangle"},0}
};
const uint8_t QUIZ_COUNT=sizeof(quizItems)/sizeof(quizItems[0]);

void drawHeader(const char* title,uint16_t accent=C_YELLOW) {
  tft.fillScreen(C_BG); tft.fillRoundRect(8,8,224,38,10,accent);
  tft.setTextColor(C_NAVY); tft.setTextSize(2); tft.setCursor(18,19); tft.print(title);
}
void drawFooter(const char* text) {
  tft.setTextSize(1); tft.setTextColor(C_NAVY); tft.setCursor(8,306); tft.print(text);
}
void drawCard(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color,const char* label,uint8_t size=2) {
  tft.fillRoundRect(x,y,w,h,9,color); tft.setTextColor(C_NAVY); tft.setTextSize(size);
  tft.setCursor(x+10,y+(h-(8*size))/2); tft.print(label);
}
void drawCentered(const char* text,int16_t y,uint16_t color=C_NAVY,uint8_t size=2) {
  tft.setTextSize(size); tft.setTextColor(color);
  int16_t x1,y1; uint16_t w,h; tft.getTextBounds(text,0,y,&x1,&y1,&w,&h);
  int16_t x=(240-(int16_t)w)/2; tft.setCursor(x<4?4:x,y); tft.print(text);
}
void drawHome() {
  tft.fillScreen(C_BG);
  // Friendly robot icon built from simple shapes.
  tft.fillRoundRect(94,12,52,42,10,C_NAVY); tft.fillRoundRect(102,22,36,24,6,C_WHITE);
  tft.fillCircle(112,33,3,C_MINT); tft.fillCircle(128,33,3,C_MINT);
  tft.drawLine(120,12,120,6,C_NAVY); tft.fillCircle(120,5,3,C_CORAL);
  drawCentered("Hello, friend!",62,C_NAVY,2); drawCentered("What shall we do?",86,C_NAVY,1);
  drawCard(12,112,104,66,C_YELLOW,"1 Learn",2); drawCard(124,112,104,66,C_MINT,"2 Play",2);
  drawCard(12,188,104,66,C_CORAL,"3 Stories",2); drawCard(124,188,104,66,C_PURPLE,"4 Quiz",2);
  tft.setTextSize(1); tft.setTextColor(C_NAVY); tft.setCursor(14,270); tft.print("Use 1-4 to choose an activity");
  tft.setCursor(14,284); tft.print("NEXT = open   BACK = home");
}
void drawLearnMenu() {
  drawHeader("Learn",C_YELLOW); const uint8_t perPage=4; uint8_t start=learnPage*perPage;
  const uint16_t colors[4]={C_YELLOW,C_MINT,C_CORAL,C_PURPLE};
  for(uint8_t i=0;i<perPage;i++){uint8_t item=start+i;if(item>=LEARN_COUNT)break;
    char label[30];snprintf(label,sizeof(label),"%u. %s",item+1,learnItems[item]);
    drawCard(12,58+i*51,216,42,colors[i],label,2);}
  tft.setTextSize(1);tft.setTextColor(C_NAVY);tft.setCursor(12,276);tft.print("Press item number; NEXT = more");
  drawFooter("BACK = home");
}
void drawNumber() {
  drawHeader("Numbers",C_YELLOW);drawCentered("Count with me!",70,C_NAVY,2);
  char num[4];snprintf(num,sizeof(num),"%u",numberValue);tft.setTextColor(C_NAVY);tft.setTextSize(8);
  int16_t x1,y1;uint16_t w,h;tft.getTextBounds(num,0,120,&x1,&y1,&w,&h);
  tft.setCursor((240-w)/2,120);tft.print(num);tft.fillRoundRect(35,230,170,40,10,C_MINT);
  drawCentered("NEXT = next number",244,C_NAVY,1);drawFooter("BACK = Learn menu");
}
void drawLearningFact(const char* title,const char* word,const char* detail,uint16_t accent) {
  drawHeader(title,accent);tft.fillRoundRect(20,70,200,112,18,accent);
  drawCentered(word,110,C_NAVY,3);drawCentered(detail,205,C_NAVY,2);drawFooter("NEXT = next   BACK = Learn");
}
void drawAbc() {
  static const char* letters[]={"A a","B b","C c","D d","E e","F f","G g","H h","I i","J j","K k","L l","M m",
    "N n","O o","P p","Q q","R r","S s","T t","U u","V v","W w","X x","Y y","Z z"};
  static const char* examples[]={"Apple","Ball","Cat","Dog","Egg","Fish","Goat","Hat","Ice","Jam","Kite","Leaf","Moon",
    "Nest","Owl","Pig","Queen","Rain","Sun","Tree","Umbrella","Van","Water","X-ray","Yak","Zebra"};
  drawHeader("ABC",C_PURPLE);drawCentered(letters[letterIndex],110,C_NAVY,5);
  drawCentered(examples[letterIndex],190,C_NAVY,2);drawFooter("NEXT = next letter   BACK = Learn");
}
void drawPlayMenu() {
  drawHeader("Play",C_MINT);drawCard(12,76,216,62,C_YELLOW,"1. Picture Match",2);
  drawCard(12,150,216,62,C_PURPLE,"2. Quick Quiz",2);tft.setTextSize(1);tft.setTextColor(C_NAVY);
  tft.setCursor(14,242);tft.print("Choose with 1 or 2");drawFooter("BACK = home");
}
void drawMatchGame() {
  const char* left[]={"CAT","SUN","FISH","TREE","BIRD"};const char* right[]={"MEOW","SHINES","SWIMS","GROWS","FLIES"};
  drawHeader("Picture Match",C_MINT);drawCard(16,72,208,54,C_YELLOW,left[matchIndex],3);
  drawCentered("matches with...",143,C_NAVY,1);drawCard(16,165,208,54,C_PURPLE,right[matchIndex],2);
  drawCentered(matchRevealed?"Great matching!":"Press NEXT to reveal",242,matchRevealed?C_GREEN:C_NAVY,1);
  drawFooter("NEXT = reveal/next   BACK = Play");
}
void drawStoriesMenu() {
  drawHeader("Stories",C_CORAL);
  for(uint8_t i=0;i<STORY_COUNT;i++){char label[30];snprintf(label,sizeof(label),"%u. %s",i+1,stories[i]);
    drawCard(12,65+i*64,216,52,i==0?C_YELLOW:(i==1?C_MINT:C_PURPLE),label,1);}
  drawFooter("1-3 choose   BACK = home");
}
void drawStoryPage() {
  drawHeader(stories[storyIndex],C_CORAL);tft.fillRoundRect(12,60,216,225,12,C_WHITE);
  uint16_t accent=storyIndex==0?C_PURPLE:(storyIndex==1?C_MINT:C_YELLOW);tft.fillRoundRect(82,72,76,48,10,accent);
  if(storyIndex==0){tft.fillCircle(120,96,14,C_YELLOW);tft.fillCircle(126,91,14,C_WHITE);}
  else if(storyIndex==1){tft.fillCircle(120,99,8,C_CORAL);tft.drawLine(120,96,120,80,C_GREEN);tft.drawLine(120,86,110,82,C_GREEN);}
  else {tft.fillCircle(120,95,15,C_GRAY);tft.fillCircle(108,85,7,C_GRAY);tft.fillCircle(132,85,7,C_GRAY);}
  tft.setTextSize(2);tft.setTextColor(C_NAVY);tft.setCursor(22,137);tft.print(storyTexts[storyIndex]);
  drawFooter("NEXT = next story   BACK = Stories");
}
void drawQuiz() {
  drawHeader("Quiz",C_PURPLE);
  if(quizIndex>=QUIZ_COUNT){drawCentered("Quiz complete!",82,C_NAVY,2);char result[24];
    snprintf(result,sizeof(result),"Score: %u / %u",score,QUIZ_COUNT);drawCentered(result,130,C_NAVY,3);
    drawCard(24,196,192,48,C_MINT,"NEXT = play again",2);drawFooter("BACK = home");return;}
  tft.setTextSize(2);tft.setTextColor(C_NAVY);tft.setCursor(14,61);tft.print(quizItems[quizIndex].question);
  const uint16_t colors[3]={C_YELLOW,C_MINT,C_CORAL};
  for(uint8_t i=0;i<3;i++){uint16_t fill=colors[i];
    if(answerChecked&&i==quizItems[quizIndex].correct)fill=C_GREEN;else if(answerChecked&&selectedAnswer==i)fill=C_CORAL;
    char label[32];snprintf(label,sizeof(label),"%u. %s",i+1,quizItems[quizIndex].answers[i]);drawCard(12,142+i*43,216,36,fill,label,2);}
  if(answerChecked)drawCentered(selectedAnswer==quizItems[quizIndex].correct?"Correct!":"Try the green answer",278,C_NAVY,1);
  else drawCentered("Press 1, 2, or 3 to answer",278,C_NAVY,1);
  drawFooter(answerChecked?"NEXT = continue   BACK = home":"BACK = home");
}
void renderScreen() {
  switch(currentScreen){
    case HOME:drawHome();break;case LEARN_MENU:drawLearnMenu();break;case NUMBERS:drawNumber();break;
    case ANIMALS:drawLearningFact("Animals","DOG","A dog can bark.",C_MINT);break;
    case COLORS:drawLearningFact("Colors","BLUE","Like the sky.",C_PURPLE);break;
    case SHAPES:drawLearningFact("Shapes","CIRCLE","Round and curved.",C_YELLOW);break;
    case FRUITS:drawLearningFact("Fruits","APPLE","Crunchy and sweet.",C_CORAL);break;
    case ABC:drawAbc();break;case PLAY_MENU:drawPlayMenu();break;case MATCH_GAME:drawMatchGame();break;
    case STORIES_MENU:drawStoriesMenu();break;case STORY_PAGE:drawStoryPage();break;case QUIZ_PAGE:drawQuiz();break;
  }
}
void goHome(){currentScreen=HOME;renderScreen();}
void openLearnItem(uint8_t item){
  if(item>=LEARN_COUNT)return;learnPage=item/4;
  switch(item){case 0:numberValue=1;currentScreen=NUMBERS;break;case 1:currentScreen=ANIMALS;break;
    case 2:currentScreen=COLORS;break;case 3:currentScreen=SHAPES;break;case 4:currentScreen=FRUITS;break;case 5:currentScreen=ABC;break;}
  renderScreen();
}
void openQuiz(){quizIndex=0;score=0;selectedAnswer=255;answerChecked=false;currentScreen=QUIZ_PAGE;renderScreen();}
void handleNumberKey(uint8_t key){
  switch(currentScreen){
    case HOME:if(key==1){currentScreen=LEARN_MENU;learnPage=0;}else if(key==2)currentScreen=PLAY_MENU;
      else if(key==3)currentScreen=STORIES_MENU;else if(key==4){openQuiz();return;}else return;renderScreen();break;
    case LEARN_MENU:if(key>=1&&key<=LEARN_COUNT)openLearnItem(key-1);break;
    case PLAY_MENU:if(key==1){matchIndex=0;matchRevealed=false;currentScreen=MATCH_GAME;renderScreen();}else if(key==2)openQuiz();break;
    case STORIES_MENU:if(key>=1&&key<=STORY_COUNT){storyIndex=key-1;currentScreen=STORY_PAGE;renderScreen();}break;
    case QUIZ_PAGE:if(quizIndex<QUIZ_COUNT&&!answerChecked&&key>=1&&key<=3){selectedAnswer=key-1;answerChecked=true;
      if(selectedAnswer==quizItems[quizIndex].correct)score++;renderScreen();}break;
    default:break;
  }
}
void handleBack(){
  switch(currentScreen){
    case HOME:return;
    case LEARN_MENU:case PLAY_MENU:case STORIES_MENU:case QUIZ_PAGE:goHome();return;
    case NUMBERS:case ANIMALS:case COLORS:case SHAPES:case FRUITS:case ABC:currentScreen=LEARN_MENU;renderScreen();return;
    case MATCH_GAME:currentScreen=PLAY_MENU;renderScreen();return;
    case STORY_PAGE:currentScreen=STORIES_MENU;renderScreen();return;
    default:goHome();return;
  }
}
void handleNext(){
  switch(currentScreen){
    case HOME:return;
    case LEARN_MENU:learnPage=(learnPage+1)%2;renderScreen();return;
    case NUMBERS:numberValue=numberValue>=20?1:numberValue+1;renderScreen();return;
    case ANIMALS:currentScreen=COLORS;renderScreen();return;
    case COLORS:currentScreen=SHAPES;renderScreen();return;
    case SHAPES:currentScreen=FRUITS;renderScreen();return;
    case FRUITS:currentScreen=ABC;renderScreen();return;
    case ABC:letterIndex=(letterIndex+1)%26;renderScreen();return;
    case PLAY_MENU:currentScreen=MATCH_GAME;matchIndex=0;matchRevealed=false;renderScreen();return;
    case MATCH_GAME:if(!matchRevealed)matchRevealed=true;else{matchIndex=(matchIndex+1)%5;matchRevealed=false;}renderScreen();return;
    case STORIES_MENU:storyIndex=0;currentScreen=STORY_PAGE;renderScreen();return;
    case STORY_PAGE:storyIndex=(storyIndex+1)%STORY_COUNT;renderScreen();return;
    case QUIZ_PAGE:if(quizIndex>=QUIZ_COUNT){quizIndex=0;score=0;selectedAnswer=255;answerChecked=false;}
      else if(answerChecked){quizIndex++;selectedAnswer=255;answerChecked=false;}renderScreen();return;
  }
}

#if LUMI_ENABLE_KEYPAD
// Layout: 1 2 3 / 4 5 6 / 7 8 9 / BACK 0 NEXT
const uint8_t KEY_MAP[4][3]={{1,2,3},{4,5,6},{7,8,9},{10,0,11}};
int8_t scanKeypad(){
  for(uint8_t r=0;r<4;r++){for(uint8_t i=0;i<4;i++)pinMode(ROW_PINS[i],INPUT);
    pinMode(ROW_PINS[r],OUTPUT);digitalWrite(ROW_PINS[r],LOW);delayMicroseconds(5);
    for(uint8_t c=0;c<3;c++)if(digitalRead(COL_PINS[c])==LOW){
      for(uint8_t i=0;i<4;i++)pinMode(ROW_PINS[i],INPUT);return KEY_MAP[r][c];}}
  for(uint8_t i=0;i<4;i++)pinMode(ROW_PINS[i],INPUT);return -1;
}
#endif

void setup(){
  Serial.begin(115200);tft.begin();tft.setRotation(0);renderScreen();Serial.println("LUMI UI scaffold started.");
#if LUMI_ENABLE_KEYPAD
  for(uint8_t i=0;i<4;i++)pinMode(ROW_PINS[i],INPUT);
  // Column pull-ups and pin choices must be verified before enabling this block.
  for(uint8_t i=0;i<3;i++)pinMode(COL_PINS[i],INPUT);
#endif
}
void loop(){
#if LUMI_ENABLE_KEYPAD
  static int8_t lastKey=-1;int8_t key=scanKeypad();
  if(key>=0&&key!=lastKey){lastKey=key;delay(35);if(scanKeypad()==key){
    if(key==10)handleBack();else if(key==11)handleNext();else handleNumberKey((uint8_t)key);}}
  else if(key<0)lastKey=-1;
#else
  delay(20);
#endif
}
