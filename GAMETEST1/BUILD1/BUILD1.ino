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

  int radius;
  int circleX;
  int circleY;

  int birdY;
  int birdX;
  int birdSize = 5;

  int pipeWidth = 10;
  int pipeHeight1 = random(20, 90);
  int pipe1X = 165;
  int pipeHeight2 = pipeHeight1 + random(-20, 20);
  int pipe2X = 185;
  int pipeHeight3 = pipeHeight2 + random(-20, 20);
  int pipe3X = 205;
  
  bool gameStatus = HIGH;
  bool gameSelected = HIGH;

//1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

unsigned long lastFrame = 0;
unsigned long screenBuffer = millis();
const int frameDelay = 120; // ~30 FPS (1000ms / 30)

// Abstract base class for a Scene
class Scene {
public:
  virtual void update() = 0;   // Game logic
  virtual void render() = 0;   // Drawing to TFT
};

// Example: Title Screen Scene
class TitleScene : public Scene {
public:
  void update() override {
    bool isUPPressed = digitalRead(buttonUP);
    bool isDOWNPressed = digitalRead(buttonDOWN);

    if (isUPPressed == LOW || isDOWNPressed == LOW)
    {
      gameSelected = !gameSelected;
    }
  }
  void render() override {
    tft.setCursor(20, 40);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    if (gameSelected == HIGH)
    {
      tft.print("Flappy Bird");
      delay(120);
      tft.setCursor(20, 40);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_BLACK);
      tft.print("Flappy Bird");    }
    else 
    {
      tft.print("Circle Test");
      delay(120);
      tft.setCursor(20, 40);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_BLACK);
      tft.print("Circle Test");
    }
  }
};

// Example: Gameplay Scene
class CircleTest : public Scene {
public:
  void update() override {
    
    bool isAPressed = digitalRead(buttonA);
    bool isBPressed = digitalRead(buttonB);
    bool isUPPressed = digitalRead(buttonUP);
    bool isDOWNPressed = digitalRead(buttonDOWN);
    bool isLEFTPressed = digitalRead(buttonLEFT);
    bool isRIGHTPressed = digitalRead(buttonRIGHT);

    if (isAPressed == LOW) {radius = radius + 1;}
    if (isBPressed == LOW)
    {
      radius = radius - 1;
      if (radius < 0) {radius = 0;}
    }
    if (isUPPressed == LOW) {circleY = circleY - 3;}
    if (isDOWNPressed == LOW) {circleY = circleY + 3;}
    if (isLEFTPressed == LOW) {circleX = circleX - 3;}
    if (isRIGHTPressed == LOW) {circleX = circleX + 3;}
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

class TheFlap : public Scene {
public:
  void update() override {
    bool isAPressed = digitalRead(buttonA);
    //if (screenBuffer - lastFrame >= frameDelay * 2)
    //{
      if(isAPressed == LOW)
      {
        birdY = birdY - 20;
      }
      birdY = birdY + 5;
    //}
    if ((birdY + 8) >= 128  || (birdY + 8) <=0)
    {
      gameStatus = LOW;
    }
    
    pipe1X = pipe1X - 3;
  }
  void render() override 
  {
    //if (screenBuffer - lastFrame >= frameDelay * 2)
    //{
      tft.fillCircle(birdX, birdY, birdSize, ST77XX_YELLOW);
      tft.fillRect(pipe1X, pipeHeight1, pipeWidth, 150, ST77XX_GREEN);
      tft.fillRect(pipe1X, pipeHeight1 - 45, pipeWidth, -150, ST77XX_GREEN);
      delay(120);
      tft.fillCircle(birdX, birdY, birdSize, ST77XX_BLACK);
      tft.fillRect(pipe1X, pipeHeight1, pipeWidth, 150, ST77XX_BLACK);
      tft.fillRect(pipe1X, pipeHeight1 - 45, pipeWidth, -150, ST77XX_BLACK);
    //} 
  }
};

class Fail : public Scene {
  public:
  void update() override {}
  void render() override
  {
    //tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(20, 40);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.print("YOU FAIL!");
    tft.setCursor(20, 60);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.print("Press Start");
    delay(120);
    tft.drawRect(20, 40, 100, 100, ST77XX_BLACK);
  }
};

// Scene Manager
Scene* currentScene;
TitleScene title;
TheFlap gameBird;
CircleTest gameTest;
Fail fail;

void setScene(Scene* newScene)
{
  currentScene = newScene;
  currentScene->render();
}

void setup(void) {
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

  radius = 0;
  birdY = tft.height()/2;
  birdX = tft.width()/3;
  circleX = tft.width()/2;
  circleY = tft.height()/2;
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
        gameStatus = HIGH;
        setScene(&title);
      }
    }
    lastState = state;
    currentScene->render();
  }
}




