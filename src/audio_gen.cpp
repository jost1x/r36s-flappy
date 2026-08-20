#include "audio_gen.h"

#include <cmath>
#include <cstring>
#include <vector>

namespace {
constexpr int kSampleRate = 22050;

Wave makeWave(int sampleCount, const std::vector<float>& samples) {
    Wave wave = {};
    wave.frameCount = sampleCount;
    wave.sampleRate = kSampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;

    std::vector<short> intSamples(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        intSamples[i] = static_cast<short>(samples[i] * 32767.0F);
    }

    wave.data = malloc(sampleCount * sizeof(short));
    std::memcpy(wave.data, intSamples.data(), sampleCount * sizeof(short));
    return wave;
}

constexpr float kMelodyNotes[] = {
    523.25F, 587.33F, 659.25F, 783.99F, 880.00F, 783.99F, 659.25F, 587.33F,
};

constexpr float kBassNotes[] = {
    261.63F, 261.63F, 329.63F, 329.63F, 392.00F, 392.00F, 329.63F, 261.63F,
};

constexpr int kNoteCount = 8;
}  // namespace

Sound generateTone(float frequency, float duration, float volume) {
    int sampleCount = static_cast<int>(kSampleRate * duration);
    std::vector<float> samples(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / kSampleRate;
        float envelope = 1.0F - (t / duration);
        envelope = envelope * envelope;
        samples[i] = std::sin(6.2832F * frequency * t) * envelope * volume;
    }
    Wave wave = makeWave(sampleCount, samples);
    Sound sound = LoadSoundFromWave(wave);
    free(wave.data);
    return sound;
}

Sound generateBgmLoop(float volume, float duration) {
    int sampleCount = static_cast<int>(kSampleRate * duration);
    float noteDuration = duration / kNoteCount;
    std::vector<float> samples(sampleCount, 0.0F);

    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / kSampleRate;
        int noteIndex = static_cast<int>(t / noteDuration) % kNoteCount;
        float noteTime = t - static_cast<float>(noteIndex) * noteDuration;
        float envelope = std::exp(-noteTime * 3.0F);

        float melodyFreq = kMelodyNotes[noteIndex];
        float melody = std::sin(6.2832F * melodyFreq * t);
        melody += std::sin(6.2832F * melodyFreq * 2.0F * t) * 0.3F;

        float bassFreq = kBassNotes[noteIndex];
        float bass = std::sin(6.2832F * bassFreq * t) * 0.6F;

        float beat = 0.0F;
        if (noteTime < 0.02F) {
            beat = (1.0F - noteTime / 0.02F) * 0.15F;
        }

        samples[i] = (melody * 0.5F + bass * 0.4F + beat) * envelope * volume;
    }

    Wave wave = makeWave(sampleCount, samples);
    Sound sound = LoadSoundFromWave(wave);
    free(wave.data);
    return sound;
}
