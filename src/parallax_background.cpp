#include "parallax_background.h"

#include <algorithm>

ParallaxBackground::ParallaxBackground() {
    for (auto& layer : layers_) {
        layer.texture = {};
        layer.speed = 0.0F;
        layer.yOffset = 0.0F;
        layer.alpha = 1.0F;
        layer.ownsTexture = false;
    }
}

ParallaxBackground::~ParallaxBackground() {
    for (auto& layer : layers_) {
        if (layer.ownsTexture && layer.texture.id != 0) {
            UnloadTexture(layer.texture);
        }
    }
}

void ParallaxBackground::update(float delta, float screenWidth) {
    for (int i = 0; i < layerCount_; ++i) {
        offsets_[i] += layers_[i].speed * delta;
        if (offsets_[i] > screenWidth) offsets_[i] -= screenWidth;
    }
}

void ParallaxBackground::draw(int screenWidth, int groundY) const {
    for (int i = 0; i < layerCount_; ++i) {
        const auto& layer = layers_[i];
        if (layer.texture.id == 0) continue;

        float texW = static_cast<float>(layer.texture.width);
        float x = offsets_[i];

        Color tint = WHITE;
        tint.a = static_cast<unsigned char>(layer.alpha * 255);

        for (float drawX = x - texW; drawX < static_cast<float>(screenWidth) + texW; drawX += texW) {
            DrawTextureEx(layer.texture, Vector2{drawX, layer.yOffset}, 0.0F, 1.0F, tint);
        }
    }

    (void)groundY;
}

void ParallaxBackground::setCloudTexture(Texture2D cloudTex) {
    if (layerCount_ < kMaxLayers) {
        layers_[layerCount_].texture = cloudTex;
        layers_[layerCount_].speed = 8.0F;
        layers_[layerCount_].yOffset = 60.0F;
        layers_[layerCount_].alpha = 0.5F;
        layers_[layerCount_].ownsTexture = false;
        layerCount_++;
    }
}
