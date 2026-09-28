#pragma once
#include "../types.h"

#define NOMINMAX
#include <windows.h>
#include <mmreg.h>

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

struct DecodedAudio {
    WAVEFORMATEX format{};  // WAVE_FORMAT_PCM, 16-bit, native channel count + sample rate
    std::vector<u8> pcm;    // Interleaved signed 16-bit PCM
};

// One-shot decoder
std::optional<DecodedAudio> decode_audio_to_pcm(std::span<const u8> bytes);

// Streaming decoder
class AudioDecoder {
public:
    AudioDecoder();
    AudioDecoder(AudioDecoder&&) noexcept;
    AudioDecoder& operator=(AudioDecoder&&) noexcept;
    ~AudioDecoder();

    AudioDecoder(const AudioDecoder&) = delete;
    AudioDecoder& operator=(const AudioDecoder&) = delete;

    // Opens a decoder over a copy of the given file bytes (the copy is retained for the decoder's life)
    static std::optional<AudioDecoder> open(std::string file_bytes);

    const WAVEFORMATEX& format() const { return m_format; }

    // Reads up to frame_count frames of interleaved s16 PCM into out. Returns the number of frames read
    u64 read(void* out, u64 frame_count);
    bool at_end() const { return m_at_end; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    WAVEFORMATEX m_format{};
    bool m_at_end = false;
};
