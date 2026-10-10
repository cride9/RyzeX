#include "RadioVisualizer.h"
#include "../../../Dependecies/BASS/bass.h"
#include <atomic>
#include <mutex>

namespace {
    using GetDataFn = DWORD(WINAPI*)(DWORD, void*, DWORD);
    using GetInfoFn = BOOL(WINAPI*)(DWORD, BASS_CHANNELINFO*);
    using IsActiveFn = DWORD(WINAPI*)(DWORD);
    GetDataFn getData = nullptr;
    GetInfoFn getInfo = nullptr;
    IsActiveFn isActive = nullptr;
    std::atomic<bool> analysisEnabled{ false };
    std::mutex frameMutex;
    radio_visualizer::Frame published;
    std::uint32_t currentChannel = 0;
    std::uint32_t sampleRate = 0;
    std::uint64_t generation = 0;
    std::uint64_t sequence = 0;

    void Publish(const radio_visualizer::Frame& frame) {
        std::lock_guard<std::mutex> lock(frameMutex);
        published = frame;
    }
}

void radio_visualizer::InitializeAnalysis() {
    getData = nullptr;
    getInfo = nullptr;
    isActive = nullptr;
    if (BASS::bass_lib_handle) {
        getData = reinterpret_cast<GetDataFn>(BASS::bass_lib.GetProcAddressFromMemory(BASS::bass_lib_handle, "BASS_ChannelGetData"));
        getInfo = reinterpret_cast<GetInfoFn>(BASS::bass_lib.GetProcAddressFromMemory(BASS::bass_lib_handle, "BASS_ChannelGetInfo"));
        isActive = reinterpret_cast<IsActiveFn>(BASS::bass_lib.GetProcAddressFromMemory(BASS::bass_lib_handle, "BASS_ChannelIsActive"));
    }
    ResetStream();
}

void radio_visualizer::ResetStream() {
    currentChannel = sampleRate = 0;
    Frame frame;
    frame.generation = ++generation;
    frame.sequence = ++sequence;
    frame.status = getData && getInfo && isActive ? AnalysisStatus::Waiting : AnalysisStatus::Unsupported;
    Publish(frame);
}

void radio_visualizer::SetAnalysisEnabled(bool enabled) { analysisEnabled.store(enabled); }
bool radio_visualizer::IsAnalysisEnabled() { return analysisEnabled.load(); }

radio_visualizer::Frame radio_visualizer::GetFrame() {
    std::lock_guard<std::mutex> lock(frameMutex);
    return published;
}

void radio_visualizer::Capture(std::uint32_t channel) {
    if (channel != currentChannel) {
        ResetStream();
        currentChannel = channel;
    }
    Frame frame;
    frame.generation = generation;
    frame.sequence = ++sequence;
    frame.status = getData && getInfo && isActive ? AnalysisStatus::Waiting : AnalysisStatus::Unsupported;
    if (!IsAnalysisEnabled() || !channel || frame.status == AnalysisStatus::Unsupported) {
        Publish(frame);
        return;
    }
    frame.status = AnalysisStatus::Stalled;
    if (!sampleRate) {
        BASS_CHANNELINFO info{};
        if (getInfo(channel, &info)) sampleRate = info.freq;
    }
    if (sampleRate >= 4000 && sampleRate <= 384000 && isActive(channel) == BASS_ACTIVE_PLAYING) {
        const DWORD read = getData(channel, frame.fft.data(), BASS_DATA_FFT2048 | BASS_DATA_FFT_REMOVEDC);
        if (read != static_cast<DWORD>(-1) && read > 0) {
            for (float& value : frame.fft) value = Bounded(value, 0.f, 4.f, 0.f);
            frame.sampleRate = sampleRate;
            frame.timestamp = Clock::now();
            frame.valid = true;
            frame.status = AnalysisStatus::Ready;
        }
    }
    Publish(frame);
}
