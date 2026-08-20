#include "game_logic.h"

#include <algorithm>

GameLogic::GameLogic() = default;

void GameLogic::resetRound() {
    bird_.reset();
    pipeManager_.reset();
    particles_.clear();
    scorePopupTimer_ = 0.0F;
    collisionFlash_ = 0.0F;
    prevScore_ = 0;
}

void GameLogic::beginRound() {
    state_ = GameState::Playing;
    bird_.flap(Config::kFlapVelocity);
}

void GameLogic::update(float delta) {
    if (state_ == GameState::Ready || state_ == GameState::GameOver) {
        bird_.update(Config::kGravity, delta);
        bird_.reset();
        bird_.update(0, 0);
        return;
    }

    if (state_ == GameState::Paused) return;

    prevScore_ = pipeManager_.getScore();

    bird_.update(Config::kGravity, delta);
    pipeManager_.update(delta, Config::kBirdX, Config::kBirdRadius, bird_.getY());

    if (pipeManager_.getScore() > prevScore_) {
        particles_.emit(Config::kBirdX, bird_.getY(), 10);
        scorePopupY_ = bird_.getY() - 30.0F;
        scorePopupTimer_ = 0.5F;
    }

    particles_.update(delta);

    if (scorePopupTimer_ > 0) scorePopupTimer_ -= delta;

    if (collisionFlash_ > 0) collisionFlash_ -= delta;
}

bool GameLogic::checkCollisions() const {
    if (pipeManager_.checkCollision(Config::kBirdX, Config::kBirdRadius, bird_.getY(), Config::kPipeWidth,
                                    pipeManager_.getCurrentGap(), Config::kGroundY)) {
        return true;
    }
    if (bird_.getY() - Config::kBirdRadius < 0.0F || bird_.getY() + Config::kBirdRadius >= Config::kGroundY) {
        return true;
    }
    return false;
}
