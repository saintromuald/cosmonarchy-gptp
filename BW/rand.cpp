#include "rand.h"
#include <random>
#include "../logger.h"

namespace bw {

static gptp::GameLogger& get_logger(LogType type) {
    switch (type) {
        case LogType::Sync:  return gptp::g_sync_random_logger;
        case LogType::Local: return gptp::g_local_random_logger;
    }
    assert(false);
    return gptp::g_sync_random_logger;
}

void RandomLogger::open() {
    m_buffer = std::stringstream{std::ios::out | std::ios::binary};

    if (!g_config.log_randoms) {
        return;
    }

    std::error_code ec;
    fs::create_directory("logs", ec);
    m_output.open("logs/randoms.bin", std::ios::out | std::ios::binary | std::ios::trunc);
}

static void write_bytes(std::ostream& os, const char* data, std::size_t size) {
    os.write(data, size);
}

static void write_bytes(std::ostream& os, const auto& data) {
    os.write(reinterpret_cast<const char*>(&data), sizeof(std::remove_reference_t<decltype(data)>));
}

static void write_bytes(std::ostream& os, const char* cstr) {
    const auto size = strlen(cstr);
    write_bytes(os, size);
    write_bytes(os, cstr, size);
}

static void write_bytes(std::ostream& os, const std::string& str) {
    write_bytes(os, str.size());
    write_bytes(os, str.data(), str.size());
}

u32 RandomLogger::log_random(LogType log_type, u32 value, RandMessage message, std::source_location src, u32 old_last_random) {
    // @Pr0nogo: Why is this commented? This disables the old log randoms which seems a mistake? -Veeq7
    // if (g_config.log_randoms) {
    //     auto& logger = get_logger(log_type);
    //     logger << src.loc.function_name() << "\n";
    //     logger << src.loc.file_name() << ":" << src.loc.line() << "\n";
    //     logger << "g_last_random_number was " << old_last_random << "\n";
    //     logger << "Randomized " << value << " on frame " << g_elapsed_time_frames << "\n\n";
    // }
    
    const auto log_randoms_to = [&](std::ostream& os) {
        write_bytes(os, u8(log_type));
        write_bytes(os, src.file_name());
        write_bytes(os, u64(src.line()));
        write_bytes(os, src.function_name());
        write_bytes(os, old_last_random);
        write_bytes(os, message.message);
    };
    if (g_config.log_randoms && m_output.is_open()) log_randoms_to(m_output);
    log_randoms_to(m_buffer);

    static int i = 0;
    bw::println("{:08}: {}", i++, enum_name_or_value(log_type));

    return value;
}

u16 random(RandMessage message, std::source_location src) {
    if (g_is_in_game_loop) {
        u32 old_last_random = g_last_random_number;
        g_last_random_number = static_cast<u32>((u64(22695477) * g_last_random_number + 1) & std::numeric_limits<u32>::max());
        return g_random_logger.log_random(LogType::Sync, (g_last_random_number >> 16) % 32768u, std::move(message), src, old_last_random);
    }
    return 0;
}

u32 rand_between(u32 min, u32 max, RandMessage message, std::source_location src) {
    assert(min <= max);
    message.assign_if_empty("rand_between {} - {}", min, max);
    return min + ((max - min + 1) * random(std::move(message), src) >> 15);
}

struct LocalRandomContext {
    std::random_device rd;
    std::mt19937 mt{rd()};
};
static LocalRandomContext g_local_random_context;

u16 random_local(RandMessage message, std::source_location src) {
    return g_random_logger.log_random(LogType::Local, g_local_random_context.mt() % 32768, std::move(message), src, 0);
}

u32 rand_between_local(u32 min, u32 max, RandMessage message, std::source_location src) {
    assert(min <= max);
    message.assign_if_empty("rand_between_local {} - {}", min, max);
    return g_random_logger.log_random(LogType::Local, g_local_random_context.mt() % (max - min + 1) + min, std::move(message), src, 3);
}

} // namespace bw
