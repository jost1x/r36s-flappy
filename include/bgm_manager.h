#pragma once

#include <raylib.h>

class BgmManager {
   public:
    BgmManager();
    ~BgmManager();

    void start();
    void stop();
    void setVolume(float volume);
    float getVolume() const { return volume_; }
    bool isPlaying() const { return playing_; }
    void toggle();

   private:
    Sound bgmSound_{};
    float volume_ = 0.15F;
    bool playing_ = false;
    bool initialized_ = false;
};
