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
    void setCloudTexture(Texture2D cloudTex);

   private:
    static constexpr int kMaxLayers = 3;
    std::array<ParallaxLayer, kMaxLayers> layers_{};
    int layerCount_ = 0;
    float offsets_[kMaxLayers] = {0.0F, 0.0F, 0.0F};
};
