#include "bgm_manager.h"

#include "audio_gen.h"

BgmManager::BgmManager() = default;

BgmManager::~BgmManager() { shutdown(); }

void BgmManager::shutdown() {
    if (initialized_) {
        StopSound(bgmSound_);
        UnloadSound(bgmSound_);
        initialized_ = false;
        playing_ = false;
    }
}

bool BgmManager::start() {
    if (!initialized_ && IsAudioDeviceReady()) {
        bgmSound_ = generateBgmLoop(1.0F);
        initialized_ = true;
    }
    if (initialized_ && !playing_) {
        PlaySound(bgmSound_);
        SetSoundVolume(bgmSound_, muted_ ? 0.0F : volume_);
        playing_ = true;
    }
    return initialized_;
}

void BgmManager::update() {
    if (initialized_ && playing_ && !IsSoundPlaying(bgmSound_)) PlaySound(bgmSound_);
}

void BgmManager::stop() {
    if (initialized_ && playing_) {
        StopSound(bgmSound_);
        playing_ = false;
    }
}

void BgmManager::setVolume(float volume) {
    volume_ = volume;
    if (initialized_) {
        SetSoundVolume(bgmSound_, muted_ ? 0.0F : volume_);
    }
}

void BgmManager::setMuted(bool muted) {
    muted_ = muted;
    if (initialized_) SetSoundVolume(bgmSound_, muted_ ? 0.0F : volume_);
}

void BgmManager::toggle() {
    if (playing_) {
        stop();
    } else {
        start();
    }
}
