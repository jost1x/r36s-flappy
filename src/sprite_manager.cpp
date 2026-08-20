#include "sprite_manager.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int kBirdSize = 48;
constexpr int kPipeBodyWidth = 76;
constexpr int kPipeBodyHeight = 256;
constexpr int kPipeLipWidth = 90;
constexpr int kPipeLipHeight = 28;
constexpr int kCloudSize = 128;
}  // namespace

SpriteManager::SpriteManager() = default;

SpriteManager::~SpriteManager() { unload(); }

void SpriteManager::load() {
    if (loaded_) return;
    generateBirdSprite();
    generatePipeSprites();
    generateCloudSprite();
    loaded_ = true;
}

void SpriteManager::unload() {
    if (birdSprite_.id != 0) UnloadTexture(birdSprite_);
    if (pipeBodySprite_.id != 0) UnloadTexture(pipeBodySprite_);
    if (pipeLipSprite_.id != 0) UnloadTexture(pipeLipSprite_);
    if (cloudSprite_.id != 0) UnloadTexture(cloudSprite_);
    birdSprite_ = {};
    pipeBodySprite_ = {};
    pipeLipSprite_ = {};
    cloudSprite_ = {};
    loaded_ = false;
}

void SpriteManager::generateBirdSprite() {
    Image img = GenImageColor(kBirdSize, kBirdSize, BLANK);

    Color body{255, 220, 45, 255};
    Color wing{245, 157, 31, 255};
    Color eyeWhite{255, 255, 255, 255};
    Color eyePupil{31, 61, 86, 255};
    Color beak{245, 107, 34, 255};
    Color outline{31, 61, 86, 255};

    int cx = kBirdSize / 2;
    int cy = kBirdSize / 2;
    float radius = 15.0F;

    for (int y = 0; y < kBirdSize; ++y) {
        for (int x = 0; x < kBirdSize; ++x) {
            int dx = x - cx;
            int dy = y - cy;
            float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));

            if (dist <= radius) {
                ImageDrawPixel(&img, x, y, body);
                if (dist <= radius + 2.0F) ImageDrawPixel(&img, x, y, outline);
            }
            if (dist <= 8 && dx < -3) {
                ImageDrawPixel(&img, x, y, wing);
            }
            if (dist <= 6 && dx > 2 && dy < -2) {
                ImageDrawPixel(&img, x, y, eyeWhite);
            }
            if (dist <= 3 && dx > 4 && dy < -2) {
                ImageDrawPixel(&img, x, y, eyePupil);
            }
            if (dx > 10 && dx < 22 && dy > -2 && dy < 6) {
                ImageDrawPixel(&img, x, y, beak);
            }
        }
    }

    birdSprite_ = LoadTextureFromImage(img);
    UnloadImage(img);
}

void SpriteManager::generatePipeSprites() {
    Color pipeGreen{73, 181, 67, 255};
    Color pipeLight{129, 216, 91, 255};
    Color pipeDark{39, 132, 52, 255};

    Image bodyImg = GenImageColor(kPipeBodyWidth, kPipeBodyHeight, pipeGreen);
    ImageDrawRectangleLines(&bodyImg,
                            Rectangle{0, 0, static_cast<float>(kPipeBodyWidth), static_cast<float>(kPipeBodyHeight)}, 3,
                            pipeDark);
    ImageDrawRectangleRec(&bodyImg, Rectangle{8, 3, 9, static_cast<float>(kPipeBodyHeight - 6)}, pipeLight);
    pipeBodySprite_ = LoadTextureFromImage(bodyImg);
    UnloadImage(bodyImg);

    Image lipImg = GenImageColor(kPipeLipWidth, kPipeLipHeight, pipeGreen);
    ImageDrawRectangleLines(
        &lipImg, Rectangle{0, 0, static_cast<float>(kPipeLipWidth), static_cast<float>(kPipeLipHeight)}, 3, pipeDark);
    ImageDrawRectangleRec(&lipImg, Rectangle{11, 3, 9, static_cast<float>(kPipeLipHeight - 6)}, pipeLight);
    pipeLipSprite_ = LoadTextureFromImage(lipImg);
    UnloadImage(lipImg);
}

void SpriteManager::generateCloudSprite() {
    Image img = GenImageColor(kCloudSize, kCloudSize, BLANK);
    Color cloud{255, 255, 255, 200};

    int cx = kCloudSize / 2;
    int cy = kCloudSize / 2;

    ImageDrawCircle(&img, cx, cy - 8, 22, cloud);
    ImageDrawCircle(&img, cx + 24, cy - 3, 17, cloud);
    ImageDrawCircle(&img, cx - 23, cy + 1, 14, cloud);
    ImageDrawRectangle(&img, cx - 38, cy, 76, 20, cloud);

    cloudSprite_ = LoadTextureFromImage(img);
    UnloadImage(img);
}
