#include "lobby_command_receive.h"
#include "hook_tools.h"
#include "BW/structures.h"
#include "BW/bwhelper.h"

namespace {
static void __declspec(naked) broadcast_player_update_wrapper() {
    static Player* player;
    __asm {
        MOV player, EDI
        PUSHAD
    }
    broadcast_player_update(*player);
    __asm {
        POPAD
        RETN
    }
}

static void __stdcall process_command_wrapper(void* data, u32 size, u32 sender_id) {
    std::span<const u8> command_data(static_cast<const u8*>(data), size);
    g_lobby_processor.process_command_data(command_data, sender_id); 
}

static void __declspec(naked) lobby_receive_wrapper() { // 00486040
    __asm {
        PUSHAD

        MOV EBX, [ESP + 0x24]   // Size
        MOV ECX, [ESP + 0x28]   // Sender
        MOV EDX, EAX            // Data

        PUSH ECX
        PUSH EBX
        PUSH EDX
        CALL process_command_wrapper

        POPAD
        RET 8
    }
}
} // namespace

namespace hooks {
void inject_lobby_command_hooks() {
    jump_patch(broadcast_player_update_wrapper, 0x00470f90);
    call_patch(lobby_receive_wrapper, 0x00486565);
    call_patch(queue_lobby_commands, 0x004f412d);
    call_patch(queue_lobby_commands, 0x004d3896);
}
} // namespace hooks