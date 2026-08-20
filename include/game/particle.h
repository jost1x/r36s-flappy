#pragma once

#include <raylib.h>

#include <array>
#include <random>

struct Particle {
    float x = 0.0F;
    float y = 0.0F;
    float vx = 0.0F;
    float vy = 0.0F;
    float life = 0.0F;
    float maxLife = 0.0F;
    Color color = WHITE;
    float size = 2.0F;
};

class ParticleSystem {
   public:
    void emit(float x, float y, int count = 8);
    void update(float delta);
    void draw() const;
    void clear();

   private:
    static constexpr int kMaxParticles = 64;
    std::array<Particle, kMaxParticles> particles_{};
    int activeCount_ = 0;
    std::mt19937 rng_{std::random_device{}()};
};
