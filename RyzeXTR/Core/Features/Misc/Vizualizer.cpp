#include "Vizualizer.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>

#pragma comment(lib, "ole32.lib")

namespace {
    constexpr float volumeThreshold = 0.65f;
    std::atomic<bool> stopRequested{ false };
    std::atomic<bool> samplingEnabled{ false };
    std::thread audioThread;
    std::mutex sampleMutex;
    float peakVolume = 0.f;
    std::uint64_t sampleNumber = 0;

    void SampleAudio() {
        if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
            return;

        IMMDeviceEnumerator* enumerator = nullptr;
        IMMDevice* device = nullptr;
        IAudioMeterInformation* meter = nullptr;
        const HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
            CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator));

        if (SUCCEEDED(result) && enumerator) {
            while (!stopRequested.load()) {
                if (!samplingEnabled.load()) {
                    Sleep(100);
                    continue;
                }

                // All COM objects belong to this worker; only sample values cross threads.
                if (!meter) {
                    if (device) {
                        device->Release();
                        device = nullptr;
                    }
                    if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device)) || !device ||
                        FAILED(device->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL,
                            nullptr, reinterpret_cast<void**>(&meter)))) {
                        Sleep(250);
                        continue;
                    }
                }

                float volume = 0.f;
                if (!meter || FAILED(meter->GetPeakValue(&volume))) {
                    if (meter) {
                        meter->Release();
                        meter = nullptr;
                    }
                    volume = 0.f;
                }
                {
                    std::lock_guard<std::mutex> lock(sampleMutex);
                    peakVolume = std::isfinite(volume) ? std::clamp(volume, 0.f, 1.f) : 0.f;
                    ++sampleNumber;
                }
                Sleep(15);
            }
        }

        if (meter) meter->Release();
        if (device) device->Release();
        if (enumerator) enumerator->Release();
        CoUninitialize();
    }
}

void vizualizer::Initialize() {
    if (audioThread.joinable())
        return;
    stopRequested.store(false);
    audioThread = std::thread(SampleAudio);
}

void vizualizer::Shutdown() {
    samplingEnabled.store(false);
    stopRequested.store(true);
    if (audioThread.joinable())
        audioThread.join();
}

void vizualizer::Draw(CBaseEntity* pLocal) {
    static std::deque<float> volumes;
    static std::uint64_t lastSample = 0;

    float volume;
    std::uint64_t number;
    {
        std::lock_guard<std::mutex> lock(sampleMutex);
        volume = peakVolume;
        number = sampleNumber;
    }

    const bool enabled = cfg::vizualizer::bEnabled && pLocal && pLocal->IsAlive();
    samplingEnabled.store(enabled);
    if (!enabled) {
        volumes.clear();
        lastSample = number;
        return;
    }
    if (number == lastSample)
        return;
    lastSample = number;

    // Configuration and entity state are accessed only on the game thread.
    bool doEffect = false;
    if (cfg::vizualizer::bDynamic) {
        volumes.push_front(volume);
        while (volumes.size() > 32)
            volumes.pop_back();
        const float peak = *std::max_element(volumes.begin(), volumes.end());
        if (peak > 0.f && volume > 0.1f && volume < volumeThreshold)
            volume *= volumeThreshold / peak;
        const float sensitivity = std::isfinite(cfg::vizualizer::flSensitivity)
            ? std::clamp(cfg::vizualizer::flSensitivity, 0.f, 0.1f) : 0.05f;
        doEffect = volume >= volumeThreshold - sensitivity;
    }
    else {
        volumes.clear();
        const float threshold = std::isfinite(cfg::vizualizer::flStaticThreshold)
            ? std::clamp(cfg::vizualizer::flStaticThreshold, 0.1f, 1.f) : 0.55f;
        doEffect = volume >= threshold;
    }

    if (doEffect)
        pLocal->HealthShotBoost() = i::GlobalVars->flCurrentTime + 0.2f;
}
