#include "parallax_background.h"

#include <algorithm>
#include <cmath>

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
        const float textureWidth = static_cast<float>(layers_[i].texture.width);
        if (textureWidth > 0.0F) {
            while (offsets_[i] >= textureWidth) offsets_[i] -= textureWidth;
        }
    }

    const float groundTileWidth = groundTexture_.id != 0 ? static_cast<float>(groundTexture_.width) : 34.0F;
    groundOffset_ += 174.0F * delta;
    while (groundOffset_ >= groundTileWidth) groundOffset_ -= groundTileWidth;

    (void)screenWidth;
}

void ParallaxBackground::drawGround(int screenWidth, int screenHeight, int groundY) const {
    if (groundTexture_.id != 0) {
        const float tileWidth = static_cast<float>(groundTexture_.width);
        for (float x = -groundOffset_ - tileWidth; x < static_cast<float>(screenWidth) + tileWidth; x += tileWidth) {
            DrawTexture(groundTexture_, static_cast<int>(std::round(x)), groundY, WHITE);
        }
        return;
    }

    DrawRectangle(0, groundY, screenWidth, screenHeight - groundY, Color{222, 211, 142, 255});
    DrawRectangle(0, groundY, screenWidth, 8, Color{82, 145, 58, 255});
    DrawRectangle(0, groundY + 2, screenWidth, 3, Color{151, 218, 90, 255});

    for (float x = -groundOffset_ - 20.0F; x < static_cast<float>(screenWidth); x += 34.0F) {
        DrawRectangle(static_cast<int>(x), groundY + 25, 19, 6, Color{191, 169, 92, 255});
    }
}

void ParallaxBackground::setGroundTexture(Texture2D groundTex) { groundTexture_ = groundTex; }

void ParallaxBackground::draw(int screenWidth, int groundY) const {
    for (int i = 0; i < layerCount_; ++i) {
        const auto& layer = layers_[i];
        if (layer.texture.id == 0) continue;

        float texW = static_cast<float>(layer.texture.width);
        float x = offsets_[i];

        Color tint = WHITE;
        tint.a = static_cast<unsigned char>(layer.alpha * 255);

        for (float drawX = x - texW; drawX < static_cast<float>(screenWidth) + texW; drawX += texW) {
            DrawTexture(layer.texture, static_cast<int>(std::round(drawX)), static_cast<int>(layer.yOffset), tint);
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

void ParallaxBackground::setSceneryTexture(Texture2D sceneryTex) {
    if (layerCount_ < kMaxLayers && sceneryTex.id != 0) {
        layers_[layerCount_].texture = sceneryTex;
        layers_[layerCount_].speed = 18.0F;
        layers_[layerCount_].yOffset = -164.0F;
        layers_[layerCount_].alpha = 1.0F;
        layers_[layerCount_].ownsTexture = false;
        layerCount_++;
        hasScenery_ = true;
    }
}
