#pragma once

#include <raylib.h>

#include <vector>

Sound generateTone(float frequency, float duration, float volume = 0.3F);

Sound generateBgmLoop(float volume, float duration = 4.0F);
