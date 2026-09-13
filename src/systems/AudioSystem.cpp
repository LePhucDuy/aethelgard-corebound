#include "systems/AudioSystem.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Cấu trúc chuẩn 44-byte RIFF/WAVE header (Little-Endian)
#pragma pack(push, 1)
struct WavHeader {
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunkSize;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4]  = {'f', 'm', 't', ' '};
    uint32_t subChunk1Size = 16;
    uint16_t audioFormat = 1;     // 1: PCM
    uint16_t numChannels = 1;     // Mono
    uint32_t sampleRate = 44100;
    uint32_t byteRate = 44100 * 1 * 2; // sampleRate * numChannels * bitsPerSample/8
    uint16_t blockAlign = 2;      // numChannels * bitsPerSample/8
    uint16_t bitsPerSample = 16;  // 16-bit
    char data[4] = {'d', 'a', 't', 'a'};
    uint32_t subChunk2Size;
};
#pragma pack(pop)

AudioSystem::AudioSystem() : initialized(false), deviceReady(false) {}

AudioSystem::~AudioSystem() {
    shutdown();
}

AudioSystem& AudioSystem::getInstance() {
    static AudioSystem instance;
    return instance;
}

void AudioSystem::init() {
    if (initialized) return;

    // Khởi tạo Audio Device của Raylib
    InitAudioDevice();
    deviceReady = IsAudioDeviceReady();

    if (!deviceReady) {
        std::cout << "[AudioSystem] Canh bao: Raylib Audio Device chua san sang. SFX se o che do im lang an toan." << std::endl;
        initialized = true;
        return;
    }

    // Tự động kiểm tra và sinh các tệp âm thanh WAV nếu chưa có
    ensureAudioAssetsExist();

    // Nạp các âm thanh vào bộ nhớ
    const std::pair<std::string, std::string> soundsToLoad[] = {
        {"sword_slash",  "assets/audio/sword_slash.wav"},
        {"hit_impact",   "assets/audio/hit_impact.wav"},
        {"parry_clash",  "assets/audio/parry_clash.wav"},
        {"shield_block", "assets/audio/shield_block.wav"},
        {"poison_tick",  "assets/audio/poison_tick.wav"},
        {"coin_pickup",  "assets/audio/coin_pickup.wav"}
    };

    for (const auto& item : soundsToLoad) {
        if (FileExists(item.second.c_str())) {
            Sound snd = LoadSound(item.second.c_str());
            soundMap[item.first] = snd;
        }
    }

    initialized = true;
    std::cout << "[AudioSystem] Khoi tao thanh cong Raylib Audio voi " << soundMap.size() << " hieu ung SFX." << std::endl;
}

void AudioSystem::play(const std::string& name, float volume, float pitch) {
    if (!deviceReady || !initialized) return;

    auto it = soundMap.find(name);
    if (it != soundMap.end()) {
        SetSoundVolume(it->second, volume);
        SetSoundPitch(it->second, pitch);
        PlaySound(it->second);
    }
}

void AudioSystem::shutdown() {
    if (!initialized) return;

    for (auto& pair : soundMap) {
        UnloadSound(pair.second);
    }
    soundMap.clear();

    if (deviceReady) {
        CloseAudioDevice();
        deviceReady = false;
    }
    initialized = false;
}

void AudioSystem::ensureAudioAssetsExist() {
    try {
        if (!fs::exists("assets/audio")) {
            fs::create_directories("assets/audio");
        }
    } catch (...) {
        // Phòng ngừa trường hợp lỗi phân quyền thư mục
    }

    struct SoundDef {
        std::string path;
        int type;
    };

    SoundDef sfxDefs[] = {
        {"assets/audio/sword_slash.wav",  0},
        {"assets/audio/hit_impact.wav",   1},
        {"assets/audio/parry_clash.wav",  2},
        {"assets/audio/shield_block.wav", 3},
        {"assets/audio/poison_tick.wav",  4},
        {"assets/audio/coin_pickup.wav",  5}
    };

    for (const auto& sfx : sfxDefs) {
        if (!fs::exists(sfx.path)) {
            generateWavFile(sfx.path, sfx.type);
        }
    }
}

void AudioSystem::generateWavFile(const std::string& filepath, int soundType) {
    constexpr uint32_t sampleRate = 44100;
    float duration = 0.2f;

    switch (soundType) {
        case 0: duration = 0.18f; break; // sword_slash
        case 1: duration = 0.16f; break; // hit_impact
        case 2: duration = 0.45f; break; // parry_clash
        case 3: duration = 0.22f; break; // shield_block
        case 4: duration = 0.20f; break; // poison_tick
        case 5: duration = 0.25f; break; // coin_pickup
        default: duration = 0.20f; break;
    }

    uint32_t numSamples = static_cast<uint32_t>(sampleRate * duration);
    std::vector<int16_t> samples(numSamples, 0);

    for (uint32_t i = 0; i < numSamples; ++i) {
        float t = (float)i / (float)sampleRate;
        float val = 0.0f;

        if (soundType == 0) {
            // SWORD SLASH: Quét tần số từ cao xuống thấp (vút kiếm) + nhiễu gió
            float freq = 850.0f - (650.0f * (t / duration));
            float env = std::sin((t / duration) * PI);
            float noise = ((float)(rand() % 2000 - 1000) / 1000.0f) * 0.35f;
            val = (std::sin(2.0f * PI * freq * t) * 0.65f + noise) * env;
        }
        else if (soundType == 1) {
            // HIT IMPACT: Lực va đập đấm / chém trầm mạnh, tiêu hao nhanh
            float freq = 180.0f * std::exp(-18.0f * t) + 60.0f;
            float env = std::exp(-20.0f * t);
            float crunch = ((float)(rand() % 1000 - 500) / 500.0f) * 0.25f * env;
            val = (std::sin(2.0f * PI * freq * t) + crunch) * env;
        }
        else if (soundType == 2) {
            // PARRY CLASH: Tiếng keng kim loại vang dội chói tai (Chuỗi bồi âm 1960Hz, 2940Hz, 4180Hz, 5250Hz)
            float env = std::exp(-7.5f * t);
            float ring = std::sin(2.0f * PI * 1960.0f * t) * 0.45f
                       + std::sin(2.0f * PI * 2940.0f * t) * 0.35f
                       + std::sin(2.0f * PI * 4180.0f * t) * 0.20f
                       + std::sin(2.0f * PI * 5250.0f * t) * 0.15f;
            float shimmer = 1.0f + 0.15f * std::sin(2.0f * PI * 22.0f * t);
            val = ring * env * shimmer;
        }
        else if (soundType == 3) {
            // SHIELD BLOCK: Tiếng đập vào khiên trầm vững
            float freq = 220.0f * std::exp(-16.0f * t) + 85.0f;
            float env = std::exp(-15.0f * t);
            val = std::sin(2.0f * PI * freq * t) * env;
        }
        else if (soundType == 4) {
            // POISON TICK: Tiếng bong bóng sủi bọt độc tố (FM modulation)
            float fCarrier = 340.0f + 180.0f * std::sin(2.0f * PI * 35.0f * t);
            float env = std::sin((t / duration) * PI);
            val = std::sin(2.0f * PI * fCarrier * t) * env;
        }
        else if (soundType == 5) {
            // COIN PICKUP: Chuông ngân 2 nốt ngọt ngào (B5 -> E6)
            float freq = (t < 0.08f) ? 987.77f : 1318.51f;
            float env = (t < 0.08f) ? (1.0f - (t / 0.08f) * 0.3f) : std::exp(-10.0f * (t - 0.08f));
            float harmonic = std::sin(2.0f * PI * freq * 2.0f * t) * 0.25f;
            val = (std::sin(2.0f * PI * freq * t) * 0.75f + harmonic) * env;
        }

        // Kẹp biên độ bảo vệ tai và tránh tràn số 16-bit
        if (val > 1.0f) val = 1.0f;
        if (val < -1.0f) val = -1.0f;
        samples[i] = static_cast<int16_t>(val * 30000.0f);
    }

    WavHeader header;
    header.subChunk2Size = numSamples * sizeof(int16_t);
    header.chunkSize = 36 + header.subChunk2Size;

    std::ofstream outFile(filepath, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
        outFile.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
        outFile.close();
        std::cout << "[AudioSystem] Da sinh thanh cong file am thanh SFX: " << filepath << std::endl;
    }
}

