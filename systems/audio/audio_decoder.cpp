#include "audio_decoder.h"

#include <miniaudio.h>

#include <utility>

namespace {
WAVEFORMATEX make_wave_format(ma_uint32 channels, ma_uint32 sample_rate) {
	constexpr u16 BitsPerSample = 16;
	const auto block_align = (u16)(channels * (BitsPerSample / 8));

	WAVEFORMATEX format{};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (u16)channels;
	format.nSamplesPerSec = sample_rate;
	format.wBitsPerSample = BitsPerSample;
	format.nBlockAlign = block_align;
	format.nAvgBytesPerSec = sample_rate * block_align;
	format.cbSize = 0;
	return format;
}
} // namespace

struct AudioDecoder::Impl {
	ma_decoder decoder{};
	std::string bytes;
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

std::optional<AudioDecoder> AudioDecoder::open(std::string file_bytes) {
	if (file_bytes.empty()) {
		return std::nullopt;
	}

	auto impl = std::make_unique<Impl>();
	impl->bytes = std::move(file_bytes);

	ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 0, 0);

	if (ma_decoder_init_memory(impl->bytes.data(), impl->bytes.size(), &config, &impl->decoder) != MA_SUCCESS) {
		return std::nullopt;
	}
	impl->initialized = true;

	ma_format format;
	ma_uint32 channels;
	ma_uint32 sample_rate;
	if (ma_decoder_get_data_format(&impl->decoder, &format, &channels, &sample_rate, nullptr, 0) != MA_SUCCESS) {
		return std::nullopt;
	}
	if (channels == 0 || sample_rate == 0) {
		return std::nullopt;
	}

	AudioDecoder result;
	result.m_impl = std::move(impl);
	result.m_format = make_wave_format(channels, sample_rate);
	return result;
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
		return std::nullopt;
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
		return std::nullopt;
	}

	return result;
}
