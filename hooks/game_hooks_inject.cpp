#include "game_hooks.h"
#include <BW/api.h>
#include <hook_tools.h>
#include <logger.h>
#include "hooks/limits/tbl_extender.h"

bool isGameOn = false;

/******** Game Start ********/

const u32 Hook_OnGameStart = 0x004C9A51;
const u32 Hook_OnGameStartBack = Hook_OnGameStart + 5;
const u32 Func_CompileSCode = 0x00417C20;
void __declspec(naked) on_game_start()
{ // From BWAPI by Kovarex, slightly modified
    __asm PUSHAD;
    {
        isGameOn = true;
        hooks::game_preinit();
        gptp::g_logger.log_game_start();
    }
    __asm {
        POPAD
        CALL Func_CompileSCode
        JMP Hook_OnGameStartBack
    }
}

/******** Game End ********/

const u32 Func_DeleteSCode = 0x00416D90;
void __declspec(naked) on_game_end()
{ // From BWAPI by Kovarex, slightly modified
    __asm PUSHAD;
    {
        isGameOn = false;
        hooks::game_end();
        gptp::g_logger.log_game_end();
        gptp::g_local_random_logger.log_end();
        gptp::g_sync_random_logger.log_end();
    }
    __asm {
        POPAD
        CALL Func_DeleteSCode
        RETN
    }
}

/******** NEXT FRAME HOOK ********/

void __declspec(naked) next_frame_hook()
{ // From BWAPI by Kovarex, slightly modified
    __asm {
        PUSHAD
        MOV EBP, ESP
    }

    {
        hooks::next_frame();
    }
    if (bw::is_game_paused()) {
        __asm {
            POPAD
            MOV EAX, 1
            RETN
        }
    } else {
        __asm {
            POPAD
            MOV EAX, 0
            RETN
        }
    }
}

void __declspec(naked) next_ui_frame_hook() {
    __asm {
        PUSHAD
        MOV EBP, ESP
    }
    hooks::next_ui_frame();
    __asm {
        POPAD
        MOV EDI, 4 // original line
        RETN
    }
}

void __declspec(naked) ui_button_click_wrapper() {
    static const u32 jmp_address = 0x004190F4;
    static BinDlg* dialog;
    __asm {
        MOV dialog, ESI
        PUSHAD
    }
    hooks::ui_button_click(dialog);
    __asm {
        POPAD
        MOV EDX, EBX
        MOV ECX, ESI
        CALL dword ptr [ESI + 0x2A]
        JMP jmp_address
    }
}

void __declspec(naked) update_doodad_visibility_wrapper() { // 00487690
    static Thingy* thingy;
    static b32 result;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        PUSH EAX
        MOV EAX, [EBP + 0x08]
        MOV thingy, EAX
        POP EAX
        PUSHAD
    }
    result = hooks::update_doodad_visibility(thingy);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        MOV EAX, result
        RETN 0x04
    }
}

void __declspec(naked) game_recv_wrapper() { // 004865D0
    static char* cur_len;
    static s32 data_len;
    static bool as_replay;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        PUSH EAX
        MOV EAX, [EBP + 0x08]
        MOV data_len, EAX
        MOV EAX, [EBP + 0x0C]
        MOV as_replay, AL
        POP EAX
        MOV cur_len, EAX
        PUSHAD
    }
    hooks::game_recv(cur_len, data_len, as_replay);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x08
    }
}

void __declspec(naked) sp_save_directory_wrapper() { // 004AE9E2
    static const u32 jmp_address = 0x004AEA15;
    static char* buffer;
    __asm {
        PUSH EAX
        LEA EAX, [EBP - 0x104]
        MOV buffer, EAX
        POP EAX
        PUSHAD
    }
    strcat(buffer, "\\");
    strcat(buffer, g_config.default_map_directory.c_str());
    __asm {
        POPAD
        JMP jmp_address
    }
}

void __declspec(naked) mp_save_directory_wrapper() { // 0044CCF7
    static const u32 jmp_address = 0x0044CD25;
    static char* buffer;
    __asm {
        MOV buffer, EDX
        PUSHAD
    }
    strcat(buffer, "\\");
    strcat(buffer, g_config.default_map_directory.c_str());
    __asm {
        POPAD
        JMP jmp_address
    }
}

void __declspec(naked) initialize_provider_version_wrapper() { // 00472770
    static GameVersion* version;
    __asm {
        MOV version, ESI
        PUSHAD
    }
    hooks::initialize_provider_version(version);
    __asm {
        POPAD
        MOV EAX, 0
        RET
    }
}

void __declspec(naked) storm_patch0() {
    static const u32 storm_Ordinal771_5ABB = 0x15005ABB;
    __asm {
        PUSH EBX
        PUSH EDI
        MOV EDI, ECX
        MOV ECX, [EDI + 0x110]
        PUSH EDI
        XOR EDI, EDI
        MOV EDX, 0x7FFFFFFF
        MOV EBX, EAX
        MOV EAX, EDX
        CMP ECX, 0x434E4C4B // 'CLNK'
        JNE offset _clnk_0
        MOV EDX, EDI

    _clnk_0:
        PUSH ESI
        MOV ESI, [EBX + 0x110]
        CMP ESI, 0x434E4C4B // 'CLNK'
        JNE offset _clnk_1
        MOV EAX, EDI

    _clnk_1:
        INC EDI
        CMP ECX, 0x434C4442 // 'CNDB'
        JNE offset _cndb_0
        MOV EDX, EDI

    _cndb_0:
        CMP ESI, 0x434C4442 // 'CNDB'
        JNE offset _cndb_1
        MOV EAX, EDI

    _cndb_1:
        INC EDI
        CMP ECX, 0x424E4554//'BNET'
        JNE offset _bnet_0
        MOV EDX, EDI

    _bnet_0:
        CMP ESI, 0x424E4554//'BNET'
        JNE offset _bnet_1
        MOV EAX, EDI

    _bnet_1:
        INC EDI
        CMP ECX, 0x4950584E//'IPXN'
        JNE offset _ipxn_0
        MOV EDX, EDI

    _ipxn_0:
        CMP ESI, 0x4950584E//'IPXN'
        JNE offset _ipxn_1
        MOV EAX, EDI

    _ipxn_1:
        INC EDI
        CMP ECX, 0x49505857//'WPXN'
        JNE offset _wpxn_0
        MOV EDX, EDI

    _wpxn_0:
        CMP ESI, 0x49505857//'WPXN'
        JNE offset _wpxn_1
        MOV EAX, EDI

    _wpxn_1:
        INC EDI
        CMP ECX, 0x4D4F444D//'MODM'
        JNE offset _modm_0
        MOV EDX, EDI

    _modm_0:
        CMP ESI, 0x4D4F444D//'MODM'
        JNE offset _modm_1
        MOV EAX, EDI

    _modm_1:
        INC EDI
        CMP ECX, 0x5343424C//'SCBL'
        JNE offset _scbl_0
        MOV EDX, EDI

    _scbl_0:
        CMP ESI, 0x5343424C//'SCBL'
        JNE offset _scbl_1
        MOV EAX, EDI

    _scbl_1:
        INC EDI
        CMP ECX, 0x4D534450//'MSDP'
        JNE offset _msdp_0
        MOV EDX, EDI

    _msdp_0:
        CMP ESI, 0x5343424C//'MSDP'
        JNE offset _msdp_1
        MOV EAX, EDI

    _msdp_1:
        POP ESI
        POP EDI
        CMP EDX, 0x7FFFFFFF
        JNE offset _546
        CMP EAX, EDX
        JNE offset _546
        ADD EBX, 0x114
        PUSH EBX
        ADD EDI, 0x114
        PUSH EDI
        CALL storm_Ordinal771_5ABB
        ADD ESP, 8
        POP EDI
        POP EBX
        RETN

    _546:
        POP EDI
        SUB EAX, EDX
        POP EBX
        RETN
    }
}

namespace hooks {
void inject_game_hooks() {
jump_patch(on_game_start,						   Hook_OnGameStart);
jump_patch(set_ingame_input_procs,			   0x00484CC0, 0);
jump_patch(update_minimap_colors,				   0x004A3A00, 0);
jump_patch(update_doodad_visibility_wrapper,	   0x00487690, 1);
jump_patch(game_recv_wrapper,					   0x004865D0, 0);

jump_patch(ui_button_click_wrapper,			   0x004190ED, 2);

call_patch(on_game_end,						   0x004EE983, 0);
call_patch(next_frame_hook,					   0x004D974E, 0);
call_patch(next_ui_frame_hook,				   0x004DE433, 0);

jump_patch(sp_save_directory_wrapper,			   0x004AE9E2, 0);
jump_patch(mp_save_directory_wrapper,			   0x0044CCF7, 1);

jump_patch(initialize_provider_version_wrapper,  0x00472770, 1);
jump_patch(network_set_return_menu,              0x004DC5B0, 0);
jump_patch(load_stat_txt,                        0x004C3A20, 1);

// Fix replay buffer overflow. (get_replay_turn_data @ 004CDFF0)
call_patch(hooks::get_replay_turn_data,          0x00487163);
call_patch(replay_loop,                          0x004D96C4);
call_patch(storm_patch0,                         0x1503E164); // DF: Re-order network providers
}
}