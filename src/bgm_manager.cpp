#include "bgm_manager.h"

#include "audio_gen.h"

BgmManager::BgmManager() = default;

BgmManager::~BgmManager() {
    if (initialized_) {
        UnloadSound(bgmSound_);
    }
}

void BgmManager::start() {
    if (!initialized_ && IsAudioDeviceReady()) {
        bgmSound_ = generateBgmLoop(volume_);
        initialized_ = true;
    }
    if (initialized_ && !playing_) {
        PlaySound(bgmSound_);
        SetSoundVolume(bgmSound_, volume_);
        playing_ = true;
    }
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
        SetSoundVolume(bgmSound_, volume_);
    }
}

void BgmManager::toggle() {
    if (playing_) {
        stop();
    } else {
        start();
    }
}
