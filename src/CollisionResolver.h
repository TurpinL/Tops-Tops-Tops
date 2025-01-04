#pragma once

#include "Vec2.h"
#include <optional>
#include "Beyblade.h"

struct Contact {
    Vec2 normal; // A unit vector represention the direction of A to B
    Vec2 position; // Midpoint of the contact
    float penetration; // Depth of penetration
    Beyblade *beyA; // Todo: Smart pointers
    Beyblade *beyB;
};

std::optional<Contact> checkCollision(Beyblade &beyA, Beyblade &beyB) {
  Vec2 midLine = (beyB.position - beyA.position);
  float distance = midLine.length();

  if (distance < (beyA.radius + beyB.radius)) {
    Contact contact;
    contact.normal = midLine.normalized();
    contact.position = beyA.position + (midLine * 0.5f);
    contact.penetration = beyA.radius + beyB.radius - distance;
    contact.beyA = &beyA;
    contact.beyB = &beyB;

    return contact;
  } else {
    return {};
  }
}

void resolveImpulseOfCollision(Contact &contact) {
  float linearClosingVelocity = contact.normal.dot(contact.beyA->velocity - contact.beyB->velocity);

  float beyASurfaceVelocity = abs(contact.beyA->calcCircumference() * contact.beyA->rpm * 60);
  float beyBSurfaceVelocity = abs(contact.beyB->calcCircumference() * contact.beyB->rpm * 60);

  // Lose some RPM in a collision
  // TODO: Some calculation that takes into account the relative RPMs 
  contact.beyA->rpm *= 0.9;
  contact.beyB->rpm *= 0.9;

  float coeffOfRestitution = 0.8f;

  contact.beyA->velocity -= contact.normal * linearClosingVelocity * coeffOfRestitution;
  // Completely made up calculation to turn lost RPM into velocity
  contact.beyA->velocity -= contact.normal.rotated((rand() % 90) - 45) * beyASurfaceVelocity * 0.00001;

  contact.beyB->velocity += contact.normal * linearClosingVelocity * coeffOfRestitution;
  contact.beyB->velocity += contact.normal.rotated((rand() % 90) - 45) * beyBSurfaceVelocity * 0.00001;
}

// Move each body out of penetration by the shortest path possible
void resolveInterpenetration(Contact &contact) {
  // Move both tops by half the interpenetration depth
  Vec2 halfPenetration = contact.normal * contact.penetration / 2;
  contact.beyA->position = contact.beyA->position - halfPenetration;
  contact.beyB->position = contact.beyB->position + halfPenetration;
}

// Collision response
/*
+ Check collisions
- Calculate the change in velocity and angular velocity from those collisions
  - Calculate the linear closing velocity (collisionNormal.dot(beyA.velocity - beyB.velocity))
  - What do we do about angular velocities, they seems like they'd always be ignored because the objects are always colliding head on
    but they shouldn't be ignored since the angular velocities of beyblades should actually contribute to a collision
- Apply those changes to the beyblades
+ Resolve interpenetration of beyblades. Separate the objects by the shortest distance possible

How does/can tilt play into all this?
*/