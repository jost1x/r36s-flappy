#pragma once

#include <raylib.h>

#include <array>

struct ParallaxLayer {
    Texture2D texture;
    float speed;
    float yOffset;
    float alpha;
    bool ownsTexture;
};

class ParallaxBackground {
   public:
    ParallaxBackground();
    ~ParallaxBackground();

    void update(float delta, float screenWidth);
    void draw(int screenWidth, int groundY) const;
    void drawGround(int screenWidth, int screenHeight, int groundY) const;
    void setCloudTexture(Texture2D cloudTex);
    void setSceneryTexture(Texture2D sceneryTex);
    void setGroundTexture(Texture2D groundTex);
    bool hasScenery() const { return hasScenery_; }

   private:
    static constexpr int kMaxLayers = 3;
    std::array<ParallaxLayer, kMaxLayers> layers_{};
    int layerCount_ = 0;
    float offsets_[kMaxLayers] = {0.0F, 0.0F, 0.0F};
    float groundOffset_ = 0.0F;
    Texture2D groundTexture_{};
    bool hasScenery_ = false;
};
