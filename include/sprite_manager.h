#pragma once

#include <raylib.h>

#include <string>

class SpriteManager {
   public:
    SpriteManager();
    ~SpriteManager();

    void load();
    void unload();

    Texture2D getBirdSprite() const { return birdSprite_; }
    Texture2D getPipeBodySprite() const { return pipeBodySprite_; }
    Texture2D getPipeLipSprite() const { return pipeLipSprite_; }
    Texture2D getCloudSprite() const { return cloudSprite_; }
    bool isLoaded() const { return loaded_; }

   private:
    void generateBirdSprite();
    void generatePipeSprites();
    void generateCloudSprite();

    Texture2D birdSprite_{};
    Texture2D pipeBodySprite_{};
    Texture2D pipeLipSprite_{};
    Texture2D cloudSprite_{};
    bool loaded_ = false;
};
