#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include <optional>
#include "utils.h"
#include "colourUtils.h"
#include "Vec2.h"
#include "Beyblade.h"
#include "CollisionResolver.h"

#define SCREEN_WIDTH 240
#define SCREEN_HALF_WIDTH 120
#define SCREEN_HEIGHT 240
#define SCREEN_HALF_HEIGHT 120

const uint16_t COLOUR_BG =        0x0000;
const uint16_t COLOUR_PRIMARY =   0xFFFF;
const uint16_t COLOUR_SECONDARY = 0xfd4f;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite screens[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};
TFT_eSprite* curScreen = &screens[0];
uint16_t* screenPtrs[2];
uint8_t curScreenIndex = 0;

Vec2 screenCenter = Vec2(SCREEN_HALF_WIDTH, SCREEN_HALF_HEIGHT);

int32_t lastFrameMillis = 0;
float fps = 0;
int32_t lastPhysicsUpdate = millis();
float physicsTickRate = 0;

const size_t beyCount = 5;
Beyblade beys[beyCount];

void render();
void renderBey(Beyblade bey);
void swapScreenBuffer();

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.initDMA();
  tft.setRotation(3);
  tft.fillScreen(COLOUR_BG);
  screenPtrs[0] = (uint16_t*)screens[0].createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  screenPtrs[1] = (uint16_t*)screens[1].createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  screens[0].setTextDatum(MC_DATUM);
  screens[1].setTextDatum(MC_DATUM);
  tft.startWrite(); // TFT chip select held low permanently
}

void loop() {
  if (!tft.dmaBusy()) {
    tft.pushImageDMA(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screenPtrs[curScreenIndex]);
    swapScreenBuffer();
    render();

    int32_t deltaMillis = millis() - lastFrameMillis;
    fps = 1000 / (float)deltaMillis;

    lastFrameMillis = millis();
  } 
}

void setup1() {
  for (Beyblade &bey : beys) {
    bey.position = Vec2((rand() % SCREEN_WIDTH) - SCREEN_HALF_WIDTH, (rand() % SCREEN_HEIGHT) - SCREEN_HALF_HEIGHT);
    bey.velocity = Vec2((rand() % SCREEN_WIDTH) - SCREEN_HALF_WIDTH, (rand() % SCREEN_HEIGHT) - SCREEN_HALF_HEIGHT);
    bey.rpm = 4000 + (rand() % 4000);
    bey.colour = rainbow(rand(), 0.4f);
  }
};

void loop1() {
  float deltaSec = (micros() - lastPhysicsUpdate) / 1000000.f;
  lastPhysicsUpdate = micros();

  physicsTickRate = 1 / deltaSec;

  for (Beyblade &bey : beys) {
    bey.stepPhysics(deltaSec);
  }

  for (int i = 0; i < beyCount; i++) {
    for (int j = i+1; j < beyCount; j++) {
      std::optional<Contact> contact = checkCollision(beys[i], beys[j]);

      if (contact.has_value()) {
        resolveInterpenetration(contact.value());
        resolveImpulseOfCollision(contact.value());
      }
    }
  }
}

////////////////////////////////////////////////////
///                   Render                    ////
////////////////////////////////////////////////////
void render() {
  curScreen->fillSprite(COLOUR_BG);

  for (Beyblade &bey : beys) {
    renderBey(bey);
  }

  // FPS
  curScreen->setTextColor(COLOUR_SECONDARY);
  curScreen->drawString(String(fps), screenCenter.x, 12, 2);
  curScreen->drawString("fps", screenCenter.x, 24, 2);

  curScreen->setTextColor(COLOUR_SECONDARY);
  curScreen->drawString(String(physicsTickRate), screenCenter.x, SCREEN_HEIGHT - 24, 2);
  curScreen->drawString("physics / sec", screenCenter.x, SCREEN_HEIGHT - 12, 2);
}

void swapScreenBuffer() {
  curScreenIndex = !curScreenIndex;
  curScreen = &screens[curScreenIndex];
}

void renderBey(Beyblade bey) { 
  Vec2 beyScreenPos = bey.position + screenCenter;
  Vec2 beyScreenForwardEdge = beyScreenPos + Vec2::fromPolar(6, bey.angle);
  Vec2 beyScreenBackEdge = beyScreenPos - Vec2::fromPolar(6, bey.angle);
  
  curScreen->drawSmoothCircle(beyScreenPos.x, beyScreenPos.y, bey.radius, bey.colour, COLOUR_BG);
  curScreen->drawCircle(beyScreenBackEdge.x, beyScreenBackEdge.y, 2, bey.colour);
  curScreen->drawCircle(beyScreenForwardEdge.x, beyScreenForwardEdge.y, 2, bey.colour);
  // curScreen->drawLine(beyScreenBackEdge.x, beyScreenBackEdge.y, beyScreenForwardEdge.x, beyScreenForwardEdge.y, COLOUR_BG);

  // for (float angle = 0; angle < 360; angle += 30) {
  //   float radians1 = (angle + bey.angle) * DEG_TO_RAD;
  //   float radians2 = (angle + 30 + bey.angle) * DEG_TO_RAD;

  //   Vec2 pos1 = Vec2(
  //     10 * cosf(radians1),
  //     10 * sinf(radians1) * cosf(bey.tilt * DEG_TO_RAD)
  //   ).rotated(bey.tiltAngle) + beyScreenPos;

  //   Vec2 pos2 = Vec2(
  //     10 * cosf(radians2),
  //     10 * sinf(radians2) * cosf(bey.tilt * DEG_TO_RAD)
  //   ).rotated(bey.tiltAngle) + beyScreenPos;

  //   // curScreen->drawPixel(pos.x, pos.y, bey.colour);
  //   curScreen->fillTriangle(
  //     pos1.x, pos1.y,
  //     pos2.x, pos2.y,
  //     beyScreenPos.x, beyScreenPos.y,
  //     0x8c71 + 0x841 * (int)(6 * cosf(bey.tiltAngle * DEG_TO_RAD) * -sinf(bey.tilt * DEG_TO_RAD))
  //   );
  // }
}