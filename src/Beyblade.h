#pragma once

#include "Vec2.h"

class Beyblade {
public:
    float radius = 10;

    Vec2 position = Vec2(0, 0);
    Vec2 velocity = Vec2(0, 0);
    
    float angle = 0;
    float rpm = 0;

    float tilt = 0;
    float tiltAngle = 0;

    void stepPhysics(float timeStepSec) {
        float angularDampingCoeff = 1 - 1/(rpm/100.f + 1); // Lower friction when it's spinning faster
        float dampingCoeff = angularDampingCoeff;

        // Acceleration towards the center of the arena
        velocity = velocity - position * 10 * timeStepSec; // TODO: Move this 

        // Update velocities
        velocity = velocity * powf(dampingCoeff, timeStepSec);
        rpm *= powf(angularDampingCoeff, timeStepSec);

        // Update position & angle
        position = position + velocity * timeStepSec;
        angle += rpm / 60.f * timeStepSec * 360;

        // Debug update tilt 
        // tiltAngle += 30 * timeStepSec;
        // tilt = 20 + 10 * sinf(millis() / 10000.f);
    }
};