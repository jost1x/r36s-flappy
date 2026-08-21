#include "sprite_manager.h"

#include <algorithm>
#include <cmath>

#ifdef EMBEDDED_ASSETS
#include "embedded_assets.h"
#endif

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
    if (!loadSpriteAssets()) {
        generateBirdSprite();
        generatePipeSprites();
    }
    generateCloudSprite();
    loaded_ = true;
}

void SpriteManager::unload() {
    for (auto& birdSprite : birdSprites_) {
        if (birdSprite.id != 0) UnloadTexture(birdSprite);
        birdSprite = {};
    }
    if (topPipeBodySprite_.id != 0) UnloadTexture(topPipeBodySprite_);
    if (topPipeLipSprite_.id != 0) UnloadTexture(topPipeLipSprite_);
    if (backgroundSprite_.id != 0) UnloadTexture(backgroundSprite_);
    if (groundSprite_.id != 0) UnloadTexture(groundSprite_);
    if (pipeBodySprite_.id != 0) UnloadTexture(pipeBodySprite_);
    if (pipeLipSprite_.id != 0) UnloadTexture(pipeLipSprite_);
    if (cloudSprite_.id != 0) UnloadTexture(cloudSprite_);
    pipeBodySprite_ = {};
    pipeLipSprite_ = {};
    topPipeBodySprite_ = {};
    topPipeLipSprite_ = {};
    backgroundSprite_ = {};
    groundSprite_ = {};
    cloudSprite_ = {};
    loaded_ = false;
}

bool SpriteManager::loadSpriteAssets() {
    Image birdImage{};
    Image backgroundImage{};
    Image groundImage{};
    Image birdWingsUpImage{};
    Image topPipeBodyImage{};
    Image topPipeLipImage{};
    Image pipeBodyImage{};
    Image pipeLipImage{};
#ifdef EMBEDDED_ASSETS
    backgroundImage = LoadImageFromMemory(".png", background_day_data, static_cast<int>(background_day_size));
    groundImage = LoadImageFromMemory(".png", ground_tile_data, static_cast<int>(ground_tile_size));
    birdImage = LoadImageFromMemory(".png", bird_down_data, static_cast<int>(bird_down_size));
    birdWingsUpImage = LoadImageFromMemory(".png", bird_up_data, static_cast<int>(bird_up_size));
    topPipeBodyImage = LoadImageFromMemory(".png", pipe_top_body_data, static_cast<int>(pipe_top_body_size));
    topPipeLipImage = LoadImageFromMemory(".png", pipe_top_lip_data, static_cast<int>(pipe_top_lip_size));
    pipeBodyImage = LoadImageFromMemory(".png", pipe_bottom_body_data, static_cast<int>(pipe_bottom_body_size));
    pipeLipImage = LoadImageFromMemory(".png", pipe_bottom_lip_data, static_cast<int>(pipe_bottom_lip_size));
#else
    if (!FileExists("assets/background_day.png") || !FileExists("assets/ground_tile.png") ||
        !FileExists("assets/bird_down.png") || !FileExists("assets/bird_up.png") ||
        !FileExists("assets/pipe_top_body.png") || !FileExists("assets/pipe_top_lip.png") ||
        !FileExists("assets/pipe_bottom_body.png") || !FileExists("assets/pipe_bottom_lip.png")) {
        return false;
    }
    backgroundImage = LoadImage("assets/background_day.png");
    groundImage = LoadImage("assets/ground_tile.png");
    birdImage = LoadImage("assets/bird_down.png");
    birdWingsUpImage = LoadImage("assets/bird_up.png");
    topPipeBodyImage = LoadImage("assets/pipe_top_body.png");
    topPipeLipImage = LoadImage("assets/pipe_top_lip.png");
    pipeBodyImage = LoadImage("assets/pipe_bottom_body.png");
    pipeLipImage = LoadImage("assets/pipe_bottom_lip.png");
#endif

    if (backgroundImage.data == nullptr || groundImage.data == nullptr || birdImage.data == nullptr ||
        birdWingsUpImage.data == nullptr || topPipeBodyImage.data == nullptr || topPipeLipImage.data == nullptr ||
        pipeBodyImage.data == nullptr || pipeLipImage.data == nullptr) {
        if (backgroundImage.data != nullptr) UnloadImage(backgroundImage);
        if (groundImage.data != nullptr) UnloadImage(groundImage);
        if (birdImage.data != nullptr) UnloadImage(birdImage);
        if (birdWingsUpImage.data != nullptr) UnloadImage(birdWingsUpImage);
        if (topPipeBodyImage.data != nullptr) UnloadImage(topPipeBodyImage);
        if (topPipeLipImage.data != nullptr) UnloadImage(topPipeLipImage);
        if (pipeBodyImage.data != nullptr) UnloadImage(pipeBodyImage);
        if (pipeLipImage.data != nullptr) UnloadImage(pipeLipImage);
        return false;
    }

    backgroundSprite_ = LoadTextureFromImage(backgroundImage);
    groundSprite_ = LoadTextureFromImage(groundImage);
    birdSprites_[0] = LoadTextureFromImage(birdImage);
    birdSprites_[1] = LoadTextureFromImage(birdWingsUpImage);
    topPipeBodySprite_ = LoadTextureFromImage(topPipeBodyImage);
    topPipeLipSprite_ = LoadTextureFromImage(topPipeLipImage);
    pipeBodySprite_ = LoadTextureFromImage(pipeBodyImage);
    pipeLipSprite_ = LoadTextureFromImage(pipeLipImage);
    UnloadImage(backgroundImage);
    UnloadImage(groundImage);
    UnloadImage(birdImage);
    UnloadImage(birdWingsUpImage);
    UnloadImage(topPipeBodyImage);
    UnloadImage(topPipeLipImage);
    UnloadImage(pipeBodyImage);
    UnloadImage(pipeLipImage);
    return backgroundSprite_.id != 0 && groundSprite_.id != 0 && birdSprites_[0].id != 0 && birdSprites_[1].id != 0 &&
           topPipeBodySprite_.id != 0 && topPipeLipSprite_.id != 0 && pipeBodySprite_.id != 0 && pipeLipSprite_.id != 0;
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

    birdSprites_[0] = LoadTextureFromImage(img);
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
    topPipeBodySprite_ = LoadTextureFromImage(bodyImg);
    UnloadImage(bodyImg);

    Image lipImg = GenImageColor(kPipeLipWidth, kPipeLipHeight, pipeGreen);
    ImageDrawRectangleLines(
        &lipImg, Rectangle{0, 0, static_cast<float>(kPipeLipWidth), static_cast<float>(kPipeLipHeight)}, 3, pipeDark);
    ImageDrawRectangleRec(&lipImg, Rectangle{11, 3, 9, static_cast<float>(kPipeLipHeight - 6)}, pipeLight);
    pipeLipSprite_ = LoadTextureFromImage(lipImg);
    topPipeLipSprite_ = LoadTextureFromImage(lipImg);
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
