#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include <optional>
#include "utils.h"
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

Beyblade bey1;
Beyblade bey2;

Contact lastContact;

void render();
void renderBey(Beyblade bey, uint16_t colour);
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
  // LED 1
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);

  bey1.position = Vec2(0, 50);
  bey1.velocity = Vec2(120, 0);
  bey1.rpm = 6000;

  bey2.position = Vec2(0, -70);
  bey2.velocity = Vec2(80, 10);
  bey2.rpm = 4000;
};

void loop1() {
  if (micros() - lastPhysicsUpdate > 1600) {
    float deltaSec = (micros() - lastPhysicsUpdate) / 1000000.f;
    physicsTickRate = 1 / deltaSec;

    bey1.stepPhysics(deltaSec);
    bey2.stepPhysics(deltaSec);

    std::optional<Contact> contact = checkCollision(bey1, bey2);

    if (contact.has_value()) {
      lastContact = contact.value();
      resolveInterpenetration(contact.value());
      resolveImpulseOfCollision(contact.value());
    }

    lastPhysicsUpdate = micros();
  }
}

////////////////////////////////////////////////////
///                   Render                    ////
////////////////////////////////////////////////////
void render() {
  curScreen->fillSprite(COLOUR_BG);

  renderBey(bey1, 0xc69f);
  renderBey(bey2, 0xfed8);

  Vec2 lastContactScreenPos = lastContact.position + screenCenter;

  curScreen->drawCircle(lastContactScreenPos.x, lastContactScreenPos.y, 2, 0xf800);
  Vec2 contactNormalEnd = lastContactScreenPos + lastContact.normal * 10;
  curScreen->drawLine(lastContactScreenPos.x, lastContactScreenPos.y, contactNormalEnd.x, contactNormalEnd.y, 0x01ff);

  Vec2 contactPenetrationStart = lastContactScreenPos + lastContact.normal.normal() * (lastContact.penetration / 2);
  Vec2 contactPenetrationEnd = lastContactScreenPos + lastContact.normal.normal() * (-lastContact.penetration / 2);
  curScreen->drawLine(contactPenetrationStart.x, contactPenetrationStart.y, contactPenetrationEnd.x, contactPenetrationEnd.y, 0x01ff);

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

void renderBey(Beyblade bey, uint16_t colour) {
  Vec2 beyScreenPos = bey.position + screenCenter;
  Vec2 beyScreenForwardEdge = beyScreenPos + Vec2::fromPolar(6, bey.angle);
  Vec2 beyScreenBackEdge = beyScreenPos - Vec2::fromPolar(6, bey.angle);
  
  curScreen->drawSpot(beyScreenPos.x, beyScreenPos.y, bey.radius, colour, COLOUR_BG);
  curScreen->drawSpot(beyScreenBackEdge.x, beyScreenBackEdge.y, 2, COLOUR_BG, colour);
  curScreen->drawSpot(beyScreenForwardEdge.x, beyScreenForwardEdge.y, 2, COLOUR_BG, colour);
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

  //   // curScreen->drawPixel(pos.x, pos.y, colour);
  //   curScreen->fillTriangle(
  //     pos1.x, pos1.y,
  //     pos2.x, pos2.y,
  //     beyScreenPos.x, beyScreenPos.y,
  //     0x8c71 + 0x841 * (int)(6 * cosf(bey.tiltAngle * DEG_TO_RAD) * -sinf(bey.tilt * DEG_TO_RAD))
  //   );
  // }
}