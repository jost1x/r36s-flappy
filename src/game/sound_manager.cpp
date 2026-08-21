#include "game/sound_manager.h"

#include "audio_gen.h"

bool SoundManager::initialize() {
    if (initialized_) return true;
    if (!IsAudioDeviceReady()) return false;
    flapSound_ = generateTone(880.0F, 0.1F, 0.2F);
    pointSound_ = generateTone(1200.0F, 0.15F, 0.25F);
    hitSound_ = generateTone(220.0F, 0.2F, 0.3F);
    initialized_ = true;
    return true;
}

SoundManager::~SoundManager() { shutdown(); }

void SoundManager::shutdown() {
    if (initialized_) {
        UnloadSound(flapSound_);
        UnloadSound(pointSound_);
        UnloadSound(hitSound_);
        initialized_ = false;
    }
}

void SoundManager::playFlap() {
    if (initialized_ && !muted_) {
        SetSoundVolume(flapSound_, volume_);
        PlaySound(flapSound_);
    }
}

void SoundManager::playPoint() {
    if (initialized_ && !muted_) {
        SetSoundVolume(pointSound_, volume_);
        PlaySound(pointSound_);
    }
}

void SoundManager::playHit() {
    if (initialized_ && !muted_) {
        SetSoundVolume(hitSound_, volume_);
        PlaySound(hitSound_);
    }
}

void SoundManager::toggleMute() { muted_ = !muted_; }

void SoundManager::setVolume(float volume) { volume_ = volume; }
