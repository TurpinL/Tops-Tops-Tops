#pragma once

#include "Vec2.h"

class Beyblade {
public:
    Vec2 position = Vec2(0, 0);
    Vec2 velocity = Vec2(0, 0);
    
    float angle = 0;
    float rpm = 0;

    float tilt = 0;
    float tiltAngle = 0;
};