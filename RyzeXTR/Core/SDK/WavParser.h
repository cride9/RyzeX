#pragma once
#include <cstddef>
#include <cstdint>

class CWavParser {
public:
    struct WavHeader_t {
        std::uint16_t m_uiFormatType = 0, m_uiChannels = 0;
        std::uint16_t m_uiBlockAlign = 0, m_uiBitsPerSample = 0;
        std::uint32_t m_uiSampleRate = 0, m_uiDataSize = 0;
        unsigned char* m_uchpData = nullptr;
        bool ParseWavHeader(unsigned char* data, std::size_t size);
    };
    bool AdjustWavVolume(WavHeader_t& header, float volume);
};
inline CWavParser wavparser;
