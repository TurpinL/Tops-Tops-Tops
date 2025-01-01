#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include "utils.h"
#include "Vec2.h"
#include "Beyblade.h"

#define SCREEN_WIDTH 240
#define SCREEN_HALF_WIDTH 120
#define SCREEN_HEIGHT 240
#define SCREEN_HALF_HEIGHT 120

const uint16_t COLOUR_BG =        0x0000;
const uint16_t COLOUR_PRIMARY =   0xFFFF;
const uint16_t COLOUR_SECONDARY = 0xfd4f;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite screen = TFT_eSprite(&tft);
uint16_t* screenPtr;

Vec2 screenCenter = Vec2(SCREEN_HALF_WIDTH, SCREEN_HALF_HEIGHT);

int32_t lastFrameMillis = 0;
float fps = 0;
Beyblade bey1;

void render();
void stepPhysics(unsigned long timeStepMillis);

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.initDMA();
  tft.setRotation(3);
  tft.fillScreen(COLOUR_BG);
  screenPtr = (uint16_t*)screen.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  screen.setTextDatum(MC_DATUM);
  tft.startWrite(); // TFT chip select held low permanently

  // LED 1
  pinMode(9, OUTPUT);

  adc_init();

  bey1.position = Vec2(12, 12);
  bey1.velocity = Vec2(-30, 0);
  bey1.rpm = 100;
}

void loop() {
  if (!tft.dmaBusy()) {
    render();

    int32_t deltaMillis = millis() - lastFrameMillis;
    fps = 100000 / deltaMillis;

    lastFrameMillis = millis();
    stepPhysics(deltaMillis);
  }
}

void stepPhysics(unsigned long timeStepMillis) {
  bey1.position = bey1.position + bey1.velocity * (timeStepMillis / 1000.f);

  // Acceleration towards center should be proportional to distance from center
  bey1.velocity = bey1.velocity - bey1.position / 10;

  bey1.rpm -= timeStepMillis / 50.f;
  bey1.angle += bey1.rpm / 60.f / 1000.f * timeStepMillis * 360;
}

////////////////////////////////////////////////////
///                   Render                    ////
////////////////////////////////////////////////////
void render() {
  screen.fillSprite(COLOUR_BG);

  Vec2 bey1ScreenPos = Vec2(bey1.position.x + screenCenter.x, bey1.position.y + screenCenter.y);
  Vec2 bey1ScreenForwardEdge = bey1ScreenPos + Vec2::fromPolar(6, bey1.angle);
  Vec2 bey1ScreenBackEdge = bey1ScreenPos - Vec2::fromPolar(6, bey1.angle);

  screen.drawSpot(bey1ScreenPos.x, bey1ScreenPos.y, 10, COLOUR_PRIMARY, COLOUR_BG);
  screen.drawSpot(bey1ScreenBackEdge.x, bey1ScreenBackEdge.y, 2, COLOUR_BG, COLOUR_PRIMARY);
  screen.drawSpot(bey1ScreenForwardEdge.x, bey1ScreenForwardEdge.y, 2, COLOUR_BG, COLOUR_PRIMARY);
  // screen.drawLine(bey1ScreenBackEdge.x, bey1ScreenBackEdge.y, bey1ScreenForwardEdge.x, bey1ScreenForwardEdge.y, COLOUR_BG);

  // FPS
  screen.setTextColor(COLOUR_SECONDARY);
  screen.drawNumber(fps, screenCenter.x, 12, 2);
  screen.drawString("fps", screenCenter.x, 24, 2);

  tft.pushImageDMA(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screenPtr);
}