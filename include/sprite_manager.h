#pragma once

#include <raylib.h>

#include <array>
#include <string>

class SpriteManager {
   public:
    SpriteManager();
    ~SpriteManager();

    void load();
    void unload();

    Texture2D getBirdSprite(bool wingsUp = false) const {
        return wingsUp && birdSprites_[1].id != 0 ? birdSprites_[1] : birdSprites_[0];
    }
    Texture2D getTopPipeBodySprite() const { return topPipeBodySprite_.id != 0 ? topPipeBodySprite_ : pipeBodySprite_; }
    Texture2D getTopPipeLipSprite() const { return topPipeLipSprite_.id != 0 ? topPipeLipSprite_ : pipeLipSprite_; }
    Texture2D getBottomPipeBodySprite() const { return pipeBodySprite_; }
    Texture2D getBottomPipeLipSprite() const { return pipeLipSprite_; }
    Texture2D getBackgroundSprite() const { return backgroundSprite_; }
    Texture2D getGroundSprite() const { return groundSprite_; }
    Texture2D getCloudSprite() const { return cloudSprite_; }
    bool isLoaded() const { return loaded_; }

   private:
    void generateBirdSprite();
    void generatePipeSprites();
    void generateCloudSprite();
    bool loadSpriteAssets();

    std::array<Texture2D, 2> birdSprites_{};
    Texture2D topPipeBodySprite_{};
    Texture2D topPipeLipSprite_{};
    Texture2D backgroundSprite_{};
    Texture2D groundSprite_{};
    Texture2D pipeBodySprite_{};
    Texture2D pipeLipSprite_{};
    Texture2D cloudSprite_{};
    bool loaded_ = false;
};
