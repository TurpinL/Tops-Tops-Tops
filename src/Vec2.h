#pragma once

#include "utils.h"

class Vec2 {
  public:
    float x;
    float y;

    Vec2() {}

    Vec2(float x, float y) {
      this->x = x;
      this->y = y;
    }

    Vec2 operator+(const Vec2& rhs) {
      return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2& rhs) {
      return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator/(const float rhs) {
      return Vec2(x / rhs, y / rhs);
    }

    Vec2 operator*(const float rhs) {
      return Vec2(x * rhs, y * rhs);
    }

    Vec2& operator+= (const Vec2& rhs)
    {
      x += rhs.x;
      y += rhs.y;
  
      return *this;
    }

    Vec2& operator-= (const Vec2& rhs)
    {
      x -= rhs.x;
      y -= rhs.y;
      
      return *this;
    }

    Vec2& operator*= (const float& rhs)
    {
      x *= rhs;
      y *= rhs;
      
      return *this;
    }

    Vec2& operator/= (const float& rhs)
    {
      x /= rhs;
      y /= rhs;

      return *this;
    }

    float length() {
      return sqrtf(x * x + y * y);
    }

    Vec2 normal() {
      return Vec2(-y, x);
    }

    Vec2 normalized() {
      return *this / length();
    }

    float dot(const Vec2 rhs) {
      return x * rhs.x + y * rhs.y;
    }

    Vec2 rotated(float degrees) {
      float radians = degrees * DEG_TO_RAD;

      return Vec2(
        x * cosf(radians) - y * sinf(radians),
        x * sinf(radians) + y * cosf(radians)
      );
    }

    // 0º is up
    static Vec2 fromPolar(float radius, float degrees) {
      float radians = (degrees - 90) * DEG_TO_RAD;
      float x = radius * cosf(radians);
      float y = radius * sinf(radians);

      return Vec2(x, y);
    }
};