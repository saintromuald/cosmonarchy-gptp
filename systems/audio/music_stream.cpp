#include "music_stream.h"

#include "audio_decoder.h"
#include "../defer.h"
#include "../BW/api.h"
#include "../storm/storm.h"

#include <algorithm>
#include <memory>
#include <utility>

static constexpr u32 MusicRingSeconds = 2;

std::unique_ptr<MusicStream> MusicStream::open(IDirectSound* direct_sound, const char* path) {
    if (!direct_sound) {
        return nullptr;
    }

    auto decoder = AudioDecoder::open_stream(path, SFileScope::DiskAndMpq);
    if (!decoder) {
        bw::println("Error: Music file {} failed to open or decode!", path);
        return nullptr;
    }

    std::unique_ptr<MusicStream> stream(new MusicStream());
    stream->m_decoder = std::move(*decoder);

    WAVEFORMATEX format = stream->m_decoder.format();
    if (format.nBlockAlign == 0) {
        return nullptr;
    }

    u32 ring_bytes = format.nAvgBytesPerSec * MusicRingSeconds;
    ring_bytes -= ring_bytes % format.nBlockAlign;
    if (ring_bytes == 0) {
        ring_bytes = format.nBlockAlign;
    }
    stream->m_ring_bytes = ring_bytes;

    DSBUFFERDESC buffer_description = {};
    buffer_description.dwSize = sizeof(DSBUFFERDESC);
    buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GETCURRENTPOSITION2;
    buffer_description.dwBufferBytes = ring_bytes;
    buffer_description.lpwfxFormat = &format;

    if (FAILED(direct_sound->CreateSoundBuffer(&buffer_description, &stream->m_ds_buffer, nullptr))) {
        return nullptr;
    }

    stream->prime();
    return stream;
}

MusicStream::~MusicStream() {
    if (m_ds_buffer) {
        m_ds_buffer->Stop();
        m_ds_buffer->Release();
        m_ds_buffer = nullptr;
    }
}

void MusicStream::fill_region(u8* destination, u32 size) {
    if (!destination || size == 0) {
        return;
    }
    if (m_decoder.at_end()) {
        std::fill_n(destination, size, (u8)0);
        return;
    }

    const u64 frames = size / block_align();
    const u64 frames_read = m_decoder.read(destination, frames);
    const u64 bytes_read = frames_read * block_align();
    m_bytes_written += bytes_read;

    if (bytes_read < size) {
        std::fill_n(destination + bytes_read, size - (u32)bytes_read, (u8)0);
    }
}

void MusicStream::prime() {
    void* ptr1 = nullptr;
    u32 size1 = 0;
    void* ptr2 = nullptr;
    u32 size2 = 0;

    if (FAILED(m_ds_buffer->Lock(0, m_ring_bytes, &ptr1, (DWORD*)&size1, &ptr2, (DWORD*)&size2, DSBLOCK_ENTIREBUFFER))) {
        return;
    }
    auto _ = utils::defer([&] { m_ds_buffer->Unlock(ptr1, size1, ptr2, size2); });

    fill_region((u8*)ptr1, size1);
    fill_region((u8*)ptr2, size2);

    m_write_offset = (u32)(m_bytes_written % m_ring_bytes);
    m_last_play_cursor = 0;
    m_bytes_played = 0;
}

void MusicStream::pump() {
    if (is_finished() || !m_ds_buffer) {
        return;
    }

    DWORD play_cursor = 0;
    if (FAILED(m_ds_buffer->GetCurrentPosition(&play_cursor, nullptr))) {
        return;
    }

    m_bytes_played += (play_cursor + m_ring_bytes - m_last_play_cursor) % m_ring_bytes;
    m_last_play_cursor = play_cursor;

    if (is_finished()) {
        return;
    }

    // Refill everything the hardware has played since our last write, kept block-aligned
    u32 writable = (play_cursor + m_ring_bytes - m_write_offset) % m_ring_bytes;
    writable -= writable % block_align();
    if (writable == 0) {
        return;
    }

    void* ptr1 = nullptr;
    u32 size1 = 0;
    void* ptr2 = nullptr;
    u32 size2 = 0;
    if (FAILED(m_ds_buffer->Lock(m_write_offset, writable, &ptr1, (DWORD*)&size1, &ptr2, (DWORD*)&size2, 0))) {
        return;
    }
    auto _ = utils::defer([&] { m_ds_buffer->Unlock(ptr1, size1, ptr2, size2); });

    fill_region((u8*)ptr1, size1);
    fill_region((u8*)ptr2, size2);

    m_write_offset = (m_write_offset + writable) % m_ring_bytes;
}

void MusicStream::play() {
    if (m_ds_buffer) {
        m_ds_buffer->Play(0, 0, DSBPLAY_LOOPING);
    }
}

void MusicStream::stop() {
    if (m_ds_buffer) {
        m_ds_buffer->Stop();
    }
}

void MusicStream::set_volume(s32 volume) {
    if (m_ds_buffer) {
        m_ds_buffer->SetVolume(volume);
    }
}

void MusicStream::set_pan(s32 pan) {
    if (m_ds_buffer) {
        m_ds_buffer->SetPan(pan);
    }
}
