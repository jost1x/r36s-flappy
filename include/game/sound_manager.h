#pragma once

#include <raylib.h>

class SoundManager {
   public:
    SoundManager() = default;
    ~SoundManager();

    bool initialize();
    void shutdown();

    void playFlap();
    void playPoint();
    void playHit();
    void toggleMute();
    void setVolume(float volume);
    bool isMuted() const { return muted_; }

   private:
    Sound flapSound_{};
    Sound pointSound_{};
    Sound hitSound_{};
    bool muted_ = false;
    bool initialized_ = false;
    float volume_ = 0.3F;
};
