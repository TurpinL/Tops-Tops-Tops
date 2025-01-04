#pragma once

#include "Vec2.h"

class Beyblade {
public:
    float radius = 10;
    uint16_t colour = 0xFFFF;

    Vec2 position = Vec2(0, 0);
    Vec2 velocity = Vec2(0, 0);
    
    float angle = 0;
    float rpm = 0;

    float tilt = 0;
    float tiltAngle = 0;

    void stepPhysics(float timeStepSec) {
        float angularDampingCoeff = 1 - 1/(abs(rpm)/100.f + 1); // Lower friction when it's spinning faster
        float dampingCoeff = angularDampingCoeff;

        // Acceleration towards the center of the arena
        velocity -= position * 10 * timeStepSec; // TODO: Move this
        
        // A little acceleration perpendicular to the arena center due to spin
        // TODO: Make this dependent on tilt
        velocity += position.normal().normalized() * rpm * 0.005 * timeStepSec;

        // Update velocities
        velocity *= powf(dampingCoeff, timeStepSec);
        rpm *= powf(angularDampingCoeff, timeStepSec);

        // Update position & angle
        position += velocity * timeStepSec;
        angle += rpm / 60.f * timeStepSec * 360;

        // Debug update tilt 
        // tiltAngle += 30 * timeStepSec;
        // tilt = 20 + 10 * sinf(millis() / 10000.f);
    }

    float calcCircumference() {
        return PI * 2 * radius;
    }
};