#include "game/particle.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kMinSpeed = 40.0F;
constexpr float kMaxSpeed = 120.0F;
constexpr float kParticleLifetime = 0.6F;
}  // namespace

void ParticleSystem::emit(float x, float y, int count) {
    std::uniform_real_distribution<float> angleDist(0.0F, 6.2832F);
    std::uniform_real_distribution<float> speedDist(kMinSpeed, kMaxSpeed);
    std::uniform_int_distribution<int> colorDist(150, 255);
    std::uniform_int_distribution<int> sizeDist(2, 4);

    for (int i = 0; i < count && activeCount_ < kMaxParticles; ++i) {
        Particle& p = particles_[activeCount_];
        p.x = x;
        p.y = y;
        float angle = angleDist(rng_);
        float speed = speedDist(rng_);
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.life = kParticleLifetime;
        p.maxLife = kParticleLifetime;
        p.color = Color{255, 255, static_cast<unsigned char>(colorDist(rng_)), 255};
        p.size = static_cast<float>(sizeDist(rng_));
        ++activeCount_;
    }
}

void ParticleSystem::update(float delta) {
    int writeIndex = 0;
    for (int i = 0; i < activeCount_; ++i) {
        Particle& p = particles_[i];
        p.x += p.vx * delta;
        p.y += p.vy * delta;
        p.vy += 200.0F * delta;
        p.life -= delta;
        if (p.life > 0) {
            if (writeIndex != i) {
                particles_[writeIndex] = p;
            }
            ++writeIndex;
        }
    }
    activeCount_ = writeIndex;
}

void ParticleSystem::draw() const {
    for (int i = 0; i < activeCount_; ++i) {
        const auto& p = particles_[i];
        float alpha = p.life / p.maxLife;
        Color c = p.color;
        c.a = static_cast<unsigned char>(alpha * 255);
        DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), p.size * alpha, c);
    }
}

void ParticleSystem::clear() { activeCount_ = 0; }
