#include "audio_decoder.h"

#include "../storm/sfile.h"

#include <miniaudio.h>

#include <utility>

namespace {
WAVEFORMATEX make_wave_format(ma_uint32 channels, ma_uint32 sample_rate) {
    constexpr u16 BitsPerSample = 16;
    const auto block_align = (u16)(channels * (BitsPerSample / 8));

    return WAVEFORMATEX{
        .wFormatTag = WAVE_FORMAT_PCM,
        .nChannels = (u16)channels,
        .nSamplesPerSec = sample_rate,
        .nAvgBytesPerSec = sample_rate * block_align,
        .nBlockAlign = block_align,
        .wBitsPerSample = BitsPerSample,
        .cbSize = 0,
    };
}

// miniaudio's ma_vfs is an opaque pointer to a struct whose first member is ma_vfs_callbacks,
// which is how the callbacks get dispatched (same layout as ma_default_vfs)
struct StormVfs {
    ma_vfs_callbacks callbacks;
    SFileScope scope;
};

sfile::StreamFile& vfs_file(ma_vfs_file file) {
    return *static_cast<sfile::StreamFile*>(file);
}

ma_result storm_vfs_open(ma_vfs* vfs, const char* path, ma_uint32 open_mode, ma_vfs_file* out_file) {
    if ((open_mode & MA_OPEN_MODE_READ) == 0) {
        return MA_INVALID_OPERATION;
    }
    const auto scope = static_cast<StormVfs*>(vfs)->scope;
    auto file = sfile::StreamFile::open(path, scope);
    if (!file) {
        return MA_DOES_NOT_EXIST;
    }
    *out_file = new sfile::StreamFile(std::move(*file));
    return MA_SUCCESS;
}

ma_result storm_vfs_close(ma_vfs* vfs, ma_vfs_file file) {
    delete &vfs_file(file);
    return MA_SUCCESS;
}

ma_result storm_vfs_read(ma_vfs* vfs, ma_vfs_file file, void* destination, size_t count, size_t* bytes_read) {
    const u64 read_bytes = vfs_file(file).read(destination, count);
    if (bytes_read) {
        *bytes_read = (size_t)read_bytes;
    }
    return MA_SUCCESS;
}

ma_result storm_vfs_seek(ma_vfs* vfs, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) {
    auto& stream = vfs_file(file);
    s64 position = offset;
    if (origin == ma_seek_origin_current) {
        position += (s64)stream.position();
    } else if (origin == ma_seek_origin_end) {
        position += (s64)stream.size();
    }
    if (position < 0) {
        return MA_INVALID_ARGS;
    }
    return stream.seek((u64)position) ? MA_SUCCESS : MA_INVALID_FILE;
}

ma_result storm_vfs_tell(ma_vfs* vfs, ma_vfs_file file, ma_int64* cursor) {
    *cursor = (ma_int64)vfs_file(file).position();
    return MA_SUCCESS;
}

ma_result storm_vfs_info(ma_vfs* vfs, ma_vfs_file file, ma_file_info* info) {
    info->sizeInBytes = vfs_file(file).size();
    return MA_SUCCESS;
}

StormVfs make_storm_vfs(SFileScope scope) {
    return StormVfs{
        .callbacks = {
            .onOpen  = storm_vfs_open,
            .onClose = storm_vfs_close,
            .onRead  = storm_vfs_read,
            .onSeek  = storm_vfs_seek,
            .onTell  = storm_vfs_tell,
            .onInfo  = storm_vfs_info,
        },
        .scope = scope,
    };
}
} // namespace

struct AudioDecoder::Impl {
    ma_decoder decoder{};
    std::string bytes; // Retained file bytes for the in-memory decoder
    StormVfs vfs = make_storm_vfs(SFileScope::DiskAndMpq);
    bool initialized = false;

    ~Impl() {
        if (initialized) {
            ma_decoder_uninit(&decoder);
        }
    }
};

AudioDecoder::AudioDecoder() = default;
AudioDecoder::AudioDecoder(AudioDecoder&&) noexcept = default;
AudioDecoder& AudioDecoder::operator=(AudioDecoder&&) noexcept = default;
AudioDecoder::~AudioDecoder() = default;

std::optional<AudioDecoder> AudioDecoder::finish_open(std::unique_ptr<Impl> impl) {
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sample_rate;
    if (ma_decoder_get_data_format(&impl->decoder, &format, &channels, &sample_rate, nullptr, 0) != MA_SUCCESS) {
        return {};
    }
    if (channels == 0 || sample_rate == 0) {
        return {};
    }

    AudioDecoder result;
    result.m_impl = std::move(impl);
    result.m_format = make_wave_format(channels, sample_rate);
    return result;
}

std::optional<AudioDecoder> AudioDecoder::open(std::string file_bytes) {
    if (file_bytes.empty()) {
        return {};
    }

    auto impl = std::make_unique<Impl>();
    impl->bytes = std::move(file_bytes);

    ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 0, 0);

    if (ma_decoder_init_memory(impl->bytes.data(), impl->bytes.size(), &config, &impl->decoder) != MA_SUCCESS) {
        return {};
    }
    impl->initialized = true;

    return finish_open(std::move(impl));
}

std::optional<AudioDecoder> AudioDecoder::open_stream(const char* path, SFileScope scope) {
    auto impl = std::make_unique<Impl>();
    impl->vfs = make_storm_vfs(scope);

    ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 0, 0);

    if (ma_decoder_init_vfs(&impl->vfs, path, &config, &impl->decoder) != MA_SUCCESS) {
        return {};
    }
    impl->initialized = true;

    return finish_open(std::move(impl));
}

u64 AudioDecoder::read(void* out, u64 frame_count) {
    if (!m_impl || frame_count == 0) {
        return 0;
    }

    ma_uint64 frames_read = 0;
    const ma_result result = ma_decoder_read_pcm_frames(&m_impl->decoder, out, frame_count, &frames_read);
    if (result != MA_SUCCESS || frames_read < frame_count) {
        m_at_end = true;
    }
    return frames_read;
}

std::optional<DecodedAudio> decode_audio_to_pcm(std::span<const u8> bytes) {
    auto decoder = AudioDecoder::open(std::string((const char*)bytes.data(), bytes.size()));
    if (!decoder) {
        return {};
    }

    DecodedAudio result;
    result.format = decoder->format();
    const size_t block_align = result.format.nBlockAlign;

    constexpr u64 ChunkFrames = 4096;
    while (!decoder->at_end()) {
        const size_t offset = result.pcm.size();
        result.pcm.resize(offset + (size_t)(ChunkFrames * block_align));

        const u64 frames_read = decoder->read(result.pcm.data() + offset, ChunkFrames);
        result.pcm.resize(offset + (size_t)(frames_read * block_align));
    }

    if (result.pcm.empty()) {
        return {};
    }

    return result;
}