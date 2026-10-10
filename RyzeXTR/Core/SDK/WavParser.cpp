#include "WavParser.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
    std::uint32_t ReadLE(const unsigned char* data, unsigned bytes) {
        std::uint32_t value = 0;
        for (unsigned i = 0; i < bytes; ++i)
            value |= static_cast<std::uint32_t>(data[i]) << (8 * i);
        return value;
    }
}

bool CWavParser::WavHeader_t::ParseWavHeader(unsigned char* data, std::size_t size) {
    *this = {};
    if (!data || size < 12 || std::memcmp(data, "RIFF", 4) || std::memcmp(data + 8, "WAVE", 4))
        return false;
    const std::size_t riffSize = ReadLE(data + 4, 4);
    if (riffSize < 4 || riffSize > size - 8)
        return false;
    const std::size_t end = riffSize + 8;
    bool formatFound = false;
    for (std::size_t offset = 12; offset + 8 <= end;) {
        const auto chunkSize = ReadLE(data + offset + 4, 4);
        auto* chunk = data + offset + 8;
        if (chunkSize > end - offset - 8)
            return false;
        if (!std::memcmp(data + offset, "fmt ", 4)) {
            if (formatFound || chunkSize < 16)
                return false;
            m_uiFormatType = static_cast<std::uint16_t>(ReadLE(chunk, 2));
            m_uiChannels = static_cast<std::uint16_t>(ReadLE(chunk + 2, 2));
            m_uiSampleRate = ReadLE(chunk + 4, 4);
            m_uiBlockAlign = static_cast<std::uint16_t>(ReadLE(chunk + 12, 2));
            m_uiBitsPerSample = static_cast<std::uint16_t>(ReadLE(chunk + 14, 2));
            formatFound = true;
        } else if (!std::memcmp(data + offset, "data", 4) && !m_uchpData) {
            m_uchpData = chunk;
            m_uiDataSize = chunkSize;
        }
        const std::size_t paddedSize = static_cast<std::size_t>(chunkSize) + (chunkSize & 1U);
        if (paddedSize > end - offset - 8)
            return false;
        offset += 8 + paddedSize;
    }
    const bool pcm = m_uiFormatType == 1 && (m_uiBitsPerSample == 8 || m_uiBitsPerSample == 16 ||
        m_uiBitsPerSample == 24 || m_uiBitsPerSample == 32);
    const bool floating = m_uiFormatType == 3 && m_uiBitsPerSample == 32;
    return formatFound && (pcm || floating) && m_uiChannels > 0 && m_uiChannels <= 8 &&
        m_uiSampleRate > 0 && m_uiSampleRate <= 384000 && m_uchpData && m_uiDataSize > 0 &&
        m_uiBlockAlign == m_uiChannels * (m_uiBitsPerSample / 8) &&
        m_uiDataSize % m_uiBlockAlign == 0;
}

bool CWavParser::AdjustWavVolume(WavHeader_t& header, float volume) {
    if (!header.m_uchpData || !header.m_uiBlockAlign || !std::isfinite(volume))
        return false;
    const unsigned bytes = header.m_uiBitsPerSample / 8;
    if ((header.m_uiFormatType != 1 && header.m_uiFormatType != 3) || bytes < 1 || bytes > 4 ||
        (header.m_uiFormatType == 3 && bytes != 4) || header.m_uiDataSize % bytes)
        return false;
    const double gain = std::clamp(volume, 0.f, 1.f);
    for (std::size_t offset = 0; offset < header.m_uiDataSize; offset += bytes) {
        auto* sample = header.m_uchpData + offset;
        if (header.m_uiFormatType == 3) {
            float value;
            std::memcpy(&value, sample, 4);
            value = std::isfinite(value) ? static_cast<float>(std::clamp(value * gain, -1.0, 1.0)) : 0.f;
            std::memcpy(sample, &value, 4);
        } else if (bytes == 1) {
            sample[0] = static_cast<unsigned char>(128 + (static_cast<int>(sample[0]) - 128) * gain);
        } else {
            const auto raw = ReadLE(sample, bytes);
            const std::int64_t signedValue = (raw & (1U << (bytes * 8 - 1)))
                ? static_cast<std::int64_t>(raw) - (1LL << (bytes * 8)) : raw;
            const auto adjusted = static_cast<std::uint32_t>(static_cast<std::int32_t>(signedValue * gain));
            for (unsigned i = 0; i < bytes; ++i)
                sample[i] = static_cast<unsigned char>(adjusted >> (i * 8));
        }
    }
    return true;
}
