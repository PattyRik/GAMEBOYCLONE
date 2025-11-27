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
//1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

unsigned long lastFrame = 0;
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
    // Check for button press or timeout to switch scene
}
  void render() override {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(20, 40);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.print("Press Start");
  }
};

// Example: Gameplay Scene
class GameScene : public Scene {
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
    tft.fillScreen(ST77XX_BLACK);
    if (radius > 0)
    {
      tft.drawCircle(circleX, circleY , radius, ST77XX_WHITE);
    } 
  }
};

// Scene Manager
Scene* currentScene;
TitleScene title;
GameScene game;

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
  circleX = tft.width()/2;
  circleY = tft.height()/2;
}

void loop() 
{
  unsigned long now = millis();
  if (now - lastFrame >= frameDelay)
  {
    lastFrame = now;
    currentScene->update();

    static bool lastState = HIGH;
    bool state = digitalRead(buttonSTART);
    if (lastState == HIGH && state == LOW)
    {
      if (currentScene == &title)
      {setScene(&game);}
      else
      {setScene(&title);}
    }
    lastState = state;
    currentScene->render();
  }
}




