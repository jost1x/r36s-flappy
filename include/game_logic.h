#pragma once

#include <raylib.h>

#include <string>

#include "config.h"
#include "game/bird.h"
#include "game/particle.h"
#include "game/pipe_manager.h"
#include "settings.h"

enum class GameState { Ready, Playing, Paused, GameOver, Options };

class GameLogic {
   public:
    GameLogic();

    void resetRound();
    void beginRound();
    void update(float delta);
    bool checkCollisions() const;

    GameState getState() const { return state_; }
    void setState(GameState state) { state_ = state; }

    Bird& getBird() { return bird_; }
    const Bird& getBird() const { return bird_; }
    PipeManager& getPipeManager() { return pipeManager_; }
    const PipeManager& getPipeManager() const { return pipeManager_; }
    ParticleSystem& getParticles() { return particles_; }
    const ParticleSystem& getParticles() const { return particles_; }

    float getScorePopupY() const { return scorePopupY_; }
    float getScorePopupTimer() const { return scorePopupTimer_; }
    float getCollisionFlash() const { return collisionFlash_; }
    int getPrevScore() const { return prevScore_; }
    bool didScore() const { return pipeManager_.getScore() > prevScore_; }

    void setScorePopupY(float y) { scorePopupY_ = y; }
    void setScorePopupTimer(float t) { scorePopupTimer_ = t; }
    void setCollisionFlash(float f) { collisionFlash_ = f; }
    void setPrevScore(int s) { prevScore_ = s; }

   private:
    GameState state_ = GameState::Ready;
    Bird bird_{Config::kBirdX, 220.0F, Config::kBirdRadius};
    PipeManager pipeManager_{
        3, Config::kPipeWidth, Config::kPipeGap, Config::kPipeSpeed, Config::kPipeSpacing, Config::kBirdX};
    ParticleSystem particles_;

    float scorePopupY_ = 0.0F;
    float scorePopupTimer_ = 0.0F;
    float collisionFlash_ = 0.0F;
    int prevScore_ = 0;
};
