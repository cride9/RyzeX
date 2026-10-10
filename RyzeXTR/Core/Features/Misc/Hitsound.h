#pragma once
#include "../../SDK/WavParser.h"
#include <Windows.h>
#include <mmsystem.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

// PlaySound(SND_MEMORY | SND_ASYNC) requires the bytes to live until playback stops.
class HitsoundCache {
    std::mutex mutex;
    std::string path, error;
    std::vector<unsigned char> original, scaled;
    float volume = -1.f;

public:
    bool Play(const std::string& requestedPath, float requestedVolume, bool reload = false) {
        std::lock_guard<std::mutex> lock(mutex);
        const float gain = std::isfinite(requestedVolume) ? std::clamp(requestedVolume, 0.f, 100.f) : 0.f;
        if (reload || requestedPath != path) {
            PlaySoundA(nullptr, nullptr, 0);
            path = requestedPath;
            volume = -1.f;
            original.clear();
            scaled.clear();
            error.clear();
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            const auto size = file ? file.tellg() : std::streampos(-1);
            if (size < std::streampos(12) || size > std::streampos(16 * 1024 * 1024)) {
                error = "WAV unavailable or invalid size (maximum 16 MB)";
                return false;
            }
            original.resize(static_cast<std::size_t>(size));
            file.seekg(0);
            CWavParser::WavHeader_t header;
            if (!file.read(reinterpret_cast<char*>(original.data()), original.size()) ||
                !header.ParseWavHeader(original.data(), original.size())) {
                original.clear();
                error = "Invalid WAV: use PCM 8/16/24/32-bit or float 32-bit";
                return false;
            }
        }
        if (original.empty())
            return false;
        if (gain != volume) {
            PlaySoundA(nullptr, nullptr, 0);
            scaled = original;
            CWavParser::WavHeader_t header;
            if (!header.ParseWavHeader(scaled.data(), scaled.size()) || !wavparser.AdjustWavVolume(header, gain / 100.f))
                return false;
            volume = gain;
        }
        if (gain == 0.f)
            return true;
        const bool success = PlaySoundA(reinterpret_cast<LPCSTR>(scaled.data()), nullptr,
            SND_MEMORY | SND_ASYNC | SND_NODEFAULT) != FALSE;
        error = success ? "" : "Could not play this WAV";
        return success;
    }

    std::string Error() {
        std::lock_guard<std::mutex> lock(mutex);
        return error;
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex);
        PlaySoundA(nullptr, nullptr, 0);
        original.clear();
        scaled.clear();
        path.clear();
        error.clear();
        volume = -1.f;
    }
};
