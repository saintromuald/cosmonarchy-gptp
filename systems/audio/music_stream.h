#pragma once
#include "../types.h"
#include "audio_decoder.h"

#define NOMINMAX
#include <mmreg.h>
#include <dsound.h>
#include <memory>

// Streams a decoded audio file into a small looping DirectSound ring buffer,
// decoding just ahead of the play cursor instead of realizing the whole track up front.
class MusicStream {
public:
    static std::unique_ptr<MusicStream> open(IDirectSound* direct_sound, const char* path);
    ~MusicStream();

    MusicStream(const MusicStream&) = delete;
    MusicStream& operator=(const MusicStream&) = delete;

    void pump(); // Decode the played - out region behind the cursor
    void play();
    void stop();
    void set_volume(s32 volume);
    void set_pan(s32 pan);
    bool is_finished() const { return m_decoder.at_end() && m_bytes_played >= m_bytes_written; }

private:
    MusicStream() = default;
    void prime();
    void fill_region(u8* destination, u32 size);
    u32 block_align() const { return m_decoder.format().nBlockAlign; }

    AudioDecoder m_decoder;
    IDirectSoundBuffer* m_ds_buffer = nullptr;
    u32 m_ring_bytes = 0;       // Cached buffer size; avoids a GetCaps COM call every pump
    u32 m_write_offset = 0;     // Ring write position; diverges from bytes_written during silence padding
    u32 m_last_play_cursor = 0; // Prior wrap cursor; required to accumulate bytes_played across wraps
    u64 m_bytes_written = 0;    // Valid PCM bytes written; freezes at stream end (excludes silence)
    u64 m_bytes_played = 0;     // Accumulated across wraps; DirectSound exposes no total-played count
};
