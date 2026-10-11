#pragma once
#include "BW/structures/UnitLayout.h"
#include "BW/bwhelper.h"
#include <exception>

inline constexpr size_t CustomCommandSize = 12;

namespace cmd {

template <typename T, u8 E>
struct Header {
    u8 type = GameCommand_Custom;
    u8 subtype = E;

    static const T& from(char* buffer) {
        if (buffer[0] != GameCommand_Custom || buffer[1] != E) {
            throw std::runtime_error{"custom command header mismatch in buffer"};
        }
        return *(T*)buffer;
    }

    void queue() {
        helper::queue_game_command(reinterpret_cast<u8*>(this), CustomCommandSize);
    }
};

struct CameraSync : Header<CameraSync, NewGameCommand::CameraSync> {
    u8 player;
    v2u16 pos;
    char unused[5];
};
static_assert(sizeof(CameraSync) == CustomCommandSize);

struct UuidRand : Header<UuidRand, NewGameCommand::UuidRand> {
    char data[10];
};
static_assert(sizeof(UuidRand) == CustomCommandSize);

} // namespace cmd
