#include <Adafruit_ImageReader.h>
#include <Adafruit_ImageReader_EPD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

  #define TFT_CS         5
  #define TFT_RST        4 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         2
  #define buttonSTART    13
  #define buttonA        12
  #define buttonB        26
  #define buttonUP       14
  #define buttonDOWN     27
  #define buttonLEFT     32
  #define buttonRIGHT    25

  const int screenW = 160;
  const int screenH = 128;

  int radius, circleX, circleY;
  
  ////////////////////////////////////////////////////////////////
  ////////////////////Flappy Bird////////////////////////////////
  int score;
  
  int birdY, birdX;
  int birdSize = 5;
  float velocity;
  const float flapImpulse = -8.0f;
  const float gravity = 1.5f;

  int pipeWidth = 12;
  int pipeGap = 45;
  int pipeHeight1, pipeHeight2, pipeHeight3, pipeHeight4;
  int pipe1X, pipe2X, pipe3X, pipe4X;
  
  bool gameStatus = HIGH;
  ////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  bool gameSelected = HIGH;

//1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

unsigned long lastFrame = 0;
unsigned long screenBuffer = millis();
const int frameDelay = 120; //  ~ (1000ms / frameDelay) fps

// Abstract base class for a Scene
class Scene {
public:
  virtual void onEnter() {};
  virtual void update() = 0;   // Game logic
  virtual void render() = 0;   // Drawing to TFT
};

// Example: Title Screen Scene
class TitleScene : public Scene {
public:
  TitleScene() : lastToggleMillis(0), needsRedraw(true) {}
  void onEnter() override {
    needsRedraw = true;
  }
  void update() override {
    bool isUPPressed = digitalRead(buttonUP) == LOW;
    bool isDOWNPressed = digitalRead(buttonDOWN) == LOW;

    unsigned long now = millis();
    const unsigned long debounceMs = 150;

    if ((isUPPressed || isDOWNPressed) && (now - lastToggleMillis > debounceMs)) {
      gameSelected = !gameSelected;        // toggle selection
      lastToggleMillis = now;              // debounce timer
      needsRedraw = true;                  // mark for redraw
    }
  }

  void render() override {
    if (!needsRedraw) return;              // nothing to do

    // Clear only the title area instead of whole screen if you prefer
    tft.fillRect(0, 30, tft.width(), 60, ST77XX_BLACK);

    tft.setCursor(10, 40);
    tft.setTextSize(2);

    if (gameSelected) {
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Flappy Bird");
      tft.setCursor(10, 70);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("Circle Test");
    } else {
      tft.setTextColor(ST77XX_WHITE);
      tft.print("Flappy Bird");
      tft.setCursor(10, 70);
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Circle Test");
    }

    needsRedraw = false;                   // rendered, clear flag
  }

private:
  bool needsRedraw;
  unsigned long lastToggleMillis;
};

// Example: Gameplay Scene
class CircleTest : public Scene 
{
public:
  void update() override {
    
    bool isAPressed = digitalRead(buttonA);
    bool isBPressed = digitalRead(buttonB);
    bool isUPPressed = digitalRead(buttonUP);
    bool isDOWNPressed = digitalRead(buttonDOWN);
    bool isLEFTPressed = digitalRead(buttonLEFT);
    bool isRIGHTPressed = digitalRead(buttonRIGHT);

    if (isAPressed == LOW) {radius ++;}
    if (isBPressed == LOW) {radius = max(0, radius - 1);}
    if (isUPPressed == LOW) {circleY -= 3;}
    if (isDOWNPressed == LOW) {circleY += 3;}
    if (isLEFTPressed == LOW) {circleX -= 3;}
    if (isRIGHTPressed == LOW) {circleX += 3;}
  }
  void render() override {
    if (radius > 0)
    {
      tft.drawCircle(circleX, circleY , radius, ST77XX_WHITE);
      delay(120);
      tft.fillCircle(circleX, circleY, radius + 1, ST77XX_BLACK);
    } 
  }
};

class FlappyBird : public Scene 
{
public:
  void update() override 
    {
      unsigned long scoreNow = millis();
      const unsigned long scoreDebounce = 500;
      //input + kinematics
      if(digitalRead(buttonA) == LOW && velocity > 2.0f) { velocity = flapImpulse; }
      velocity += gravity; 
      birdY += (int)velocity;
      //clamp + FallFail
      if ((birdY + birdSize) >= screenH) { gameStatus = LOW; }
      if ((birdY - birdSize) <= 0) { birdY = birdSize + 1; velocity = 0;}
      //Pipes movement and wrapping
      pipe1X -= 3; pipe2X -= 3; pipe3X -= 3; pipe4X -= 3;
      if (pipe1X <= -pipeWidth)
      {
        pipe1X = pipe4X + 70;
        pipeHeight1 = random(55, 115);
      }
      if (pipe2X <= -pipeWidth)
      {
        pipe2X = pipe1X + 70;
        pipeHeight2 = random(55, 115);
      }
      if (pipe3X <= -pipeWidth)
      {
        pipe3X = pipe2X + 70;
        pipeHeight3 = random(55, 115);
      }
      if (pipe4X <= -pipeWidth)
      {
        pipe4X = pipe3X + 70;
        pipeHeight4 = random(55, 115);
      }
      //collsions
      if (checkCollisions(pipe1X, pipeHeight1) || checkCollisions(pipe2X, pipeHeight2) || checkCollisions(pipe3X, pipeHeight3) || checkCollisions(pipe4X, pipeHeight4)) { gameStatus = LOW; }
      //score
      if (birdX - birdSize > pipe1X && birdX - birdSize < pipe1X + pipeWidth && scoreNow - lastToggle > scoreDebounce) {score ++; lastToggle = scoreNow;}
      if (birdX - birdSize > pipe2X && birdX - birdSize < pipe2X + pipeWidth && scoreNow - lastToggle > scoreDebounce) {score ++; lastToggle = scoreNow;}
      if (birdX - birdSize > pipe3X && birdX - birdSize < pipe3X + pipeWidth && scoreNow - lastToggle > scoreDebounce) {score ++; lastToggle = scoreNow;}
      if (birdX - birdSize > pipe4X && birdX - birdSize < pipe4X + pipeWidth && scoreNow - lastToggle > scoreDebounce) {score ++; lastToggle = scoreNow;}
    }
  void render() override 
    {
      tft.fillCircle(birdX, birdY, birdSize, ST77XX_YELLOW);
      tft.fillRect(pipe1X, pipeHeight1, pipeWidth, 150, ST77XX_GREEN);
      tft.fillRect(pipe1X, 0, pipeWidth, pipeHeight1 - pipeGap, ST77XX_GREEN);
      tft.fillRect(pipe2X, pipeHeight2, pipeWidth, 150, ST77XX_GREEN);
      tft.fillRect(pipe2X, 0, pipeWidth, pipeHeight2 - pipeGap, ST77XX_GREEN);
      tft.fillRect(pipe3X, pipeHeight3, pipeWidth, 150, ST77XX_GREEN);
      tft.fillRect(pipe3X, 0, pipeWidth, pipeHeight3 - pipeGap, ST77XX_GREEN);
      tft.fillRect(pipe4X, pipeHeight4, pipeWidth, 150, ST77XX_GREEN);
      tft.fillRect(pipe4X, 0, pipeWidth, pipeHeight4 - pipeGap, ST77XX_GREEN);
      tft.setCursor(5, screenH - 18);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(score);
      delay(120);
      tft.fillCircle(birdX, birdY, birdSize, ST77XX_BLACK);
      tft.fillRect(pipe1X, pipeHeight1, pipeWidth, 150, ST77XX_BLACK);
      tft.fillRect(pipe1X, 0, pipeWidth, pipeHeight1 - pipeGap, ST77XX_BLACK);
      tft.fillRect(pipe2X, pipeHeight2, pipeWidth, 150, ST77XX_BLACK);
      tft.fillRect(pipe2X, 0, pipeWidth, pipeHeight2 - pipeGap, ST77XX_BLACK);
      tft.fillRect(pipe3X, pipeHeight3, pipeWidth, 150, ST77XX_BLACK);
      tft.fillRect(pipe3X, 0, pipeWidth, pipeHeight3 - pipeGap, ST77XX_BLACK);
      tft.fillRect(pipe4X, pipeHeight4, pipeWidth, 150, ST77XX_BLACK);
      tft.fillRect(pipe4X, 0, pipeWidth, pipeHeight4 - pipeGap, ST77XX_BLACK);
      tft.setCursor(5, screenH - 18);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_BLACK);
      tft.print(score);
    }
private:
  bool checkCollisions(int px, int pHeight) 
    {
      //check X collisions
      int px2 = px + pipeWidth;
      int birdLeft = birdX - birdSize; int birdRight = birdX + birdSize;
      bool overlapX = !(birdRight < px || birdLeft > px2);
      if (!overlapX) return LOW;//if the bird is not within the x values associated with the pipes area we can be certain a collision cant take place
      //check Y collisions
      int pipeBottom = pHeight; int pipeTop = pHeight - pipeGap;
      int birdTop = birdY - birdSize; int birdBottom = birdY + birdSize;
      bool overlapY = !(birdTop > pipeTop && birdBottom < pipeBottom);
      if (overlapY) return HIGH; 
      return LOW;
    }
  unsigned long lastToggle;
};

class Fail : public Scene 
{
  public:
  void update() override {}
  void render() override
  {
    tft.setCursor(20, 40);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.print("YOU FAIL!");
    tft.setCursor(20, 60);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.print("Press Start");
  }
};

// Scene Manager
Scene* currentScene;
TitleScene title;
FlappyBird gameBird;
CircleTest gameTest;
Fail fail;

void setScene(Scene* newScene)
{
  currentScene = newScene;
  currentScene->onEnter();
  currentScene->render();
}

void setup(void) 
{
  Serial.begin(115200);
  
  Serial.print(F("Hello! ST77xx TFT Test"));
  randomSeed(analogRead(15));
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(1);
  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  //button Setting
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonUP, INPUT_PULLUP);
  pinMode(buttonDOWN, INPUT_PULLUP);
  pinMode(buttonLEFT, INPUT_PULLUP);
  pinMode(buttonRIGHT, INPUT_PULLUP);
  pinMode(buttonSTART, INPUT_PULLUP);

  tft.fillScreen(ST77XX_BLACK);
  setScene(&title);

  radius = 5;

  score = 0;

  birdY = tft.height()/2;
  birdX = tft.width()/3;
  circleX = tft.width()/2;
  circleY = tft.height()/2;
  
  pipeHeight1 = random(55, 115);
  pipe1X = 165;
  pipeHeight2 = 64 + random(-9, 51);
  pipe2X = 235;
  pipeHeight3 = 64 + random(-9, 51);
  pipe3X = 305;
  pipeHeight4 = 64 + random(-9, 51);
  pipe4X = 375;
}

void loop() 
{
  screenBuffer = millis();
  if (screenBuffer - lastFrame >= frameDelay)
  {
    lastFrame = screenBuffer;
    currentScene->update();

    static bool lastState = HIGH;
    bool state = digitalRead(buttonSTART);
    if (gameStatus == LOW) {setScene(&fail);}

    if (lastState == HIGH && state == LOW)
    {
      if (currentScene == &title)
      {
        switch(gameSelected) {
        case HIGH:
        tft.fillScreen(ST77XX_BLACK);
        gameStatus == HIGH;
        setScene(&gameBird);
        break;
        case LOW:
        tft.fillScreen(ST77XX_BLACK);
        setScene(&gameTest);
        break;
        }
      }
      else if (currentScene == &gameBird || currentScene == &gameTest)
      {
        tft.fillScreen(ST77XX_BLACK);
        setScene(&title); 
      }
      
      else
      {
        tft.fillScreen(ST77XX_BLACK);
        
        birdY = tft.height() / 2;
        velocity = 0;
        pipeHeight1 = random(50, 115);
        pipe1X = 165;
        pipeHeight2 = random(50, 115);
        pipe2X = 235;
        pipeHeight3 = random(50, 115);
        pipe3X = 305;
        pipeHeight4 = random(50, 115);
        pipe4X = 375;
        
        score = 0;
        gameStatus = HIGH;
        setScene(&title);
      }
    }
    lastState = state;
    currentScene->render();
  }
}
