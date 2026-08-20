#pragma once

#include <raylib.h>

class Bird {
   public:
    Bird(float x, float startY, float radius);

    void reset();
    void flap(float velocity);
    void update(float gravity, float delta);
    void draw() const;

    float getY() const { return y_; }
    float getVelocity() const { return velocity_; }
    float getRadius() const { return radius_; }
    float getRotation() const { return rotation_; }
    float getWingOffset() const { return wingOffset_; }

   private:
    float x_;
    float startY_;
    float y_;
    float velocity_;
    float radius_;
    float rotation_;
    float wingOffset_;
    float wingPhase_;
};
