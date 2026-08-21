#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "game/bird.h"
#include "game/particle.h"
#include "game/pipe_manager.h"
#include "menu_input.h"
#include "settings.h"

void testBirdReset() {
    Bird bird(100.0F, 200.0F, 15.0F);
    bird.flap(-330.0F);
    bird.update(920.0F, 0.5F);
    bird.reset();
    assert(std::abs(bird.getY() - 200.0F) < 0.01F);
    assert(std::abs(bird.getVelocity()) < 0.01F);
    assert(std::abs(bird.getRotation()) < 0.01F);
    std::cout << "testBirdReset passed\n";
}

void testBirdFlap() {
    Bird bird(100.0F, 200.0F, 15.0F);
    bird.flap(-330.0F);
    assert(std::abs(bird.getVelocity() - (-330.0F)) < 0.01F);
    std::cout << "testBirdFlap passed\n";
}

void testBirdGravity() {
    Bird bird(100.0F, 200.0F, 15.0F);
    float gravity = 920.0F;
    float delta = 0.1F;
    bird.update(gravity, delta);
    float expectedVelocity = gravity * delta;
    assert(std::abs(bird.getVelocity() - expectedVelocity) < 0.01F);
    std::cout << "testBirdGravity passed\n";
}

void testBirdRotation() {
    Bird bird(100.0F, 200.0F, 15.0F);
    bird.flap(-330.0F);
    bird.update(920.0F, 0.05F);
    assert(bird.getRotation() < 0);
    std::cout << "testBirdRotation passed\n";
}

void testBirdRotationFalling() {
    Bird bird(100.0F, 200.0F, 15.0F);
    for (int i = 0; i < 20; ++i) {
        bird.update(920.0F, 0.1F);
    }
    assert(bird.getRotation() > 0);
    std::cout << "testBirdRotationFalling passed\n";
}

void testPipeManagerReset() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    assert(manager.getScore() == 0);
    assert(manager.getLevel() == 0);
    std::cout << "testPipeManagerReset passed\n";
}

void testPipeManagerScoring() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    while (manager.getScore() < 10) {
        manager.update(1.0F, birdX, birdRadius, birdY);
    }
    assert(manager.getScore() >= 10);
    std::cout << "testPipeManagerScoring passed\n";
}

void testPipeManagerLevelUp() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    while (manager.getLevel() < 2) {
        manager.update(1.0F, birdX, birdRadius, birdY);
    }
    assert(manager.getLevel() >= 2);
    assert(manager.getPipeSpeed() > 174.0F);
    std::cout << "testPipeManagerLevelUp passed\n";
}

void testPipeManagerGapReduction() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    float initialGap = manager.getCurrentGap();
    while (manager.getLevel() < 3) {
        manager.update(1.0F, birdX, birdRadius, birdY);
    }
    assert(manager.getCurrentGap() < initialGap);
    std::cout << "testPipeManagerGapReduction passed\n";
}

void testPipeManagerMinGap() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    while (manager.getLevel() < 20) {
        manager.update(1.0F, birdX, birdRadius, birdY);
    }
    assert(manager.getCurrentGap() >= 120.0F);
    std::cout << "testPipeManagerMinGap passed\n";
}

void testPipeManagerCollision() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    bool collision = manager.checkCollision(birdX, birdRadius, birdY, 76.0F, 154.0F, 426.0F);
    assert(!collision);
    std::cout << "testPipeManagerCollision passed\n";
}

void testParticleSystemEmit() {
    ParticleSystem particles;
    particles.emit(100.0F, 100.0F, 8);
    particles.update(0.0F);
    std::cout << "testParticleSystemEmit passed\n";
}

void testParticleSystemUpdate() {
    ParticleSystem particles;
    particles.emit(100.0F, 100.0F, 8);
    particles.update(1.0F);
    std::cout << "testParticleSystemUpdate passed\n";
}

void testParticleSystemClear() {
    ParticleSystem particles;
    particles.emit(100.0F, 100.0F, 8);
    particles.clear();
    std::cout << "testParticleSystemClear passed\n";
}

void testCollisionBirdAtPipeEdge() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    bool noCollision = manager.checkCollision(birdX, birdRadius, birdY, 76.0F, 154.0F, 426.0F);
    assert(!noCollision);

    float extremeY = 10.0F;
    bool collisionExtreme = manager.checkCollision(birdX, birdRadius, extremeY, 76.0F, 154.0F, 426.0F);
    assert(!collisionExtreme);
    std::cout << "testCollisionBirdAtPipeEdge passed\n";
}

void testCollisionBirdAtGapBottom() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    bool noCollision = manager.checkCollision(birdX, birdRadius, birdY, 76.0F, 154.0F, 426.0F);
    assert(!noCollision);
    std::cout << "testCollisionBirdAtGapBottom passed\n";
}

void testCollisionBirdCenteredInGap() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;

    const auto& pipes = manager.getPipes();
    float gapCenter = pipes[0].gapCenter;

    bool collision = manager.checkCollision(birdX, birdRadius, gapCenter, 76.0F, 154.0F, 426.0F);
    assert(!collision);
    std::cout << "testCollisionBirdCenteredInGap passed\n";
}

void testCollisionBirdPastPipe() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;
    float birdY = 220.0F;

    while (manager.getScore() < 1) {
        manager.update(1.0F, birdX, birdRadius, birdY);
    }

    bool collision = manager.checkCollision(birdX, birdRadius, birdY, 76.0F, 154.0F, 426.0F);
    assert(!collision);
    std::cout << "testCollisionBirdPastPipe passed\n";
}

void testCollisionMultiplePipes() {
    PipeManager manager(3, 76.0F, 154.0F, 174.0F, 225.0F, 176.0F);
    float birdX = 176.0F;
    float birdRadius = 15.0F;

    const auto& pipes = manager.getPipes();
    bool collision = manager.checkCollision(birdX, birdRadius, 100.0F, 76.0F, 154.0F, 426.0F);

    (void)pipes;
    assert(!collision);
    std::cout << "testCollisionMultiplePipes passed\n";
}

void testParticleMaxCapacity() {
    ParticleSystem particles;
    particles.emit(100.0F, 100.0F, 100);
    particles.update(0.0F);
    std::cout << "testParticleMaxCapacity passed\n";
}

void testParticleLifetime() {
    ParticleSystem particles;
    particles.emit(100.0F, 100.0F, 8);
    particles.update(1.0F);
    std::cout << "testParticleLifetime passed\n";
}

void testMenuActions() {
    InputActions noController{};
    assert(resolveMenuAction(GameState::Ready, 0, noController) == UiAction::None);
    assert(wrapMenuSelection(0, 2, -1) == 1);
    assert(wrapMenuSelection(1, 2, 1) == 0);

    InputActions confirm{};
    confirm.confirm = true;
    assert(resolveMenuAction(GameState::Ready, 0, confirm) == UiAction::Play);
    assert(resolveMenuAction(GameState::Ready, 1, confirm) == UiAction::Options);
    assert(resolveMenuAction(GameState::Paused, 1, confirm) == UiAction::Restart);

    InputActions back{};
    back.back = true;
    assert(resolveMenuAction(GameState::Options, 0, back) == UiAction::Back);
    assert(resolveMenuAction(GameState::GameOver, 0, back) == UiAction::Menu);

    InputActions pause{};
    pause.pause = true;
    assert(resolveMenuAction(GameState::Paused, 0, pause) == UiAction::Continue);
    std::cout << "testMenuActions passed\n";
}

void testSettingsLoad() {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "r36s-flappy-settings-test.ini";
    std::filesystem::remove(path);
    GameSettings missing = GameSettings::loadFromPath(path.string());
    assert(std::abs(missing.sfxVolume - 0.3F) < 0.01F);

    {
        std::ofstream file(path);
        file << "sfx_volume=bad\n";
        file << "bgm_volume=2.5\n";
        file << "vibration=invalid\n";
    }
    GameSettings corrupt = GameSettings::loadFromPath(path.string());
    assert(std::abs(corrupt.sfxVolume - 0.3F) < 0.01F);
    assert(std::abs(corrupt.bgmVolume - 1.0F) < 0.01F);
    assert(corrupt.vibrationEnabled);
    std::filesystem::remove(path);
    std::cout << "testSettingsLoad passed\n";
}

int main() {
    testBirdReset();
    testBirdFlap();
    testBirdGravity();
    testBirdRotation();
    testBirdRotationFalling();
    testPipeManagerReset();
    testPipeManagerScoring();
    testPipeManagerLevelUp();
    testPipeManagerGapReduction();
    testPipeManagerMinGap();
    testPipeManagerCollision();
    testCollisionBirdAtPipeEdge();
    testCollisionBirdAtGapBottom();
    testCollisionBirdCenteredInGap();
    testCollisionBirdPastPipe();
    testCollisionMultiplePipes();
    testParticleSystemEmit();
    testParticleSystemUpdate();
    testParticleSystemClear();
    testParticleMaxCapacity();
    testParticleLifetime();
    testMenuActions();
    testSettingsLoad();
    std::cout << "All game tests passed\n";
    return 0;
}
