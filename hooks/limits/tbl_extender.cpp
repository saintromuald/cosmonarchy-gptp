#include "tbl_extender.h"
#include "storm/storm.h"
#include "systems/locale/locale.h"
#include <string>
#include <format>

namespace hooks {

template <typename... Ts>
static void load_stat_txt_error(const std::format_string<Ts...>& format, Ts&&... args) {
    //const auto msg = std::vformat(format, std::make_format_args(std::forward<Ts>(args)...));
    const auto msg = std::format(format, std::forward<Ts>(args)...);
    MessageBoxA(0, msg.c_str(), "Load Stat Txt Error", MB_OK | MB_ICONHAND);
    exit(-1);
}

void __cdecl load_stat_txt() { // 004C3A20
    static constexpr auto RezStatTxt = "rez\\stat.txt";
    auto base_lines = locale::read_stat_file(RezStatTxt);
    if (!base_lines) {
        load_stat_txt_error("File {} not found", RezStatTxt);
    }
    auto lines = std::move(*base_lines);
    locale::apply_translations(lines);

    size_t data_size = 0;
    for (const auto& line : lines) {
        data_size += line.size() + 1; // + null char
    }

    size_t header_size = sizeof(u32) + lines.size() * sizeof(u32);
    g_stat_txt_ptr = (u32*)SMemAlloc(header_size + data_size); // Allocates to zeroes, so null chars are implicit
    g_stat_txt_ptr[0] = lines.size();

    char* data_ptr = (char*)g_stat_txt_ptr + header_size;
    for (size_t i = 0; i < lines.size(); i++) {
        const auto& line = lines[i];
        std::ranges::copy(line, data_ptr);
        size_t offset = data_ptr - ((char*)g_stat_txt_ptr);
        g_stat_txt_ptr[i+1] = offset;
        data_ptr += line.size() + 1; // + null char
    }
}

static const char* __fastcall get_string(u32 index) {
    __asm PUSH EDX
    const char* result = g_stat_txt_tbl->get_string32(index);
    __asm POP EDX
    return result;
}
 
static const char* __fastcall get_string_edx(u32, u32 index) {
    __asm PUSH ECX
    const char* result = g_stat_txt_tbl->get_string32(index);
    __asm POP ECX
    return result;
}

void inject_tbl_extender() {
    jump_patch(load_stat_txt, 0x004C3A20, 1);
 
    static constexpr u8 mov_ecx = 0xB9;
    static constexpr u8 mov_edx = 0xBA;
    static constexpr u32 lea_ecx_eax1 = 0x0001488D;
    static constexpr u32 lea_edx_eax1 = 0x0001508D;
    static constexpr u16 mov_ebx_eax = 0xD88B;
    static constexpr u16 mov_ecx_eax = 0xC88B;
    static constexpr u16 mov_edx_eax = 0xD08B;
    static constexpr u16 mov_edi_eax = 0xF88B;
    static constexpr u16 mov_esi_eax = 0xF08B;
     
    // CMDACT_Stimpack @ 4234D0
    memory_patch(0x004234F6, mov_ecx);
    memory_patch(0x004234F7, (u32)874);
    call_patch(get_string, 0x004234FB, 8);
     
    // stats_text_status_ranking @ 425B50
    memory_patch(0x00425B7E, mov_ecx);
    memory_patch(0x00425B7F, (u32)832);
    call_patch(get_string, 0x00425B83, 8);
    memory_patch(0x00425C51, mov_ecx);
    memory_patch(0x00425C52, (u32)1300);
    call_patch(get_string, 0x00425C56, 8);
    memory_patch(0x00425CAE, mov_ecx);
    memory_patch(0x00425CAF, (u32)1300);
    call_patch(get_string, 0x00425CB3, 8);
    memory_patch(0x00425D02, mov_ecx);
    memory_patch(0x00425D03, (u32)827);
    call_patch(get_string, 0x00425D07, 8);
    memory_patch(0x00425D52, mov_ecx);
    memory_patch(0x00425D53, (u32)831);
    call_patch(get_string, 0x00425D57, 8);
     
    // stats_text_kills @ 425DD0
    memory_patch(0x00425E3A, mov_ecx);
    memory_patch(0x00425E3B, (u32)832);
    call_patch(get_string, 0x00425E3F, 8);
     
    // draw_addon_progress @ 425F30
    memory_patch(0x0042607D, lea_ecx_eax1);
    call_patch(get_string, 0x00426080, 8);
     
    // draw_construction_progress @ 4260C0
    memory_patch(0x0042614E, lea_ecx_eax1);
    call_patch(get_string, 0x00426151, 8);
     
    // stats_text_geyser @ 426190
    memory_patch(0x0042620D, mov_ecx);
    memory_patch(0x0042620E, (u32)796);
    call_patch(get_string, 0x00426212, 1);
    nop_patch(0x0042621F, 2);
    nop_patch(0x00426224, 5);
    memory_patch(0x00426247, mov_ecx);
    memory_patch(0x00426248, (u32)797);
    call_patch(get_string, 0x0042624C, 8);
    memory_patch(0x00426283, mov_ecx);
    memory_patch(0x00426284, (u32)798);
    call_patch(get_string, 0x00426288, 8);
     
    // draw_upgrade_research @ 426500
    call_patch(get_string, 0x004266AB, 11);
     
    // draw_tech_research @ 4266F0
    call_patch(get_string, 0x0042688E, 11);
     
    // draw_train_progress @ 4268D0
    memory_patch(0x00426BAF, lea_ecx_eax1);
    call_patch(get_string, 0x00426BB2);
    memory_patch(0x00426BB7, mov_edx_eax);
    nop_patch(0x00426BB9, 6);
     
    // draw_nuke_progress @ 426FF0
    memory_patch(0x00427188, lea_ecx_eax1);
    call_patch(get_string, 0x0042718B, 8);
     
    // UnitStatAct_Egg @ 427260
    memory_patch(0x0042738C, lea_ecx_eax1);
    call_patch(get_string, 0x0042738F, 8);
     
    // UnitStatAct_Archon @ 4273E0
    memory_patch(0x00427449, mov_ecx);
    memory_patch(0x0042744A, (u32)790);
    call_patch(get_string, 0x0042744E, 9);
     
    // drawSupplyUnitStatus @ 427540
    memory_patch(0x00427624, lea_ecx_eax1);
    call_patch(get_string, 0x00427627);
    memory_patch(0x0042762C, mov_ecx_eax);
    nop_patch(0x0042762E, 6);
    memory_patch(0x0042767B, lea_ecx_eax1);
    call_patch(get_string, 0x0042767E);
    memory_patch(0x00427683, mov_esi_eax);
    nop_patch(0x00427685, 6);
    memory_patch(0x004276EF, (u8)0x41);//INC ECX
    call_patch(get_string, 0x004276F0);
    memory_patch(0x004276F5, mov_esi_eax);
    nop_patch(0x004276F7, 8);
    memory_patch(0x0042779A, lea_edx_eax1);
    call_patch(get_string_edx, 0x0042779D, 8);
     
    // replayStatBtns @ 427A80
    memory_patch(0x00427ACF, mov_ecx);
    memory_patch(0x00427AD0, (u32)1545);
    nop_patch(0x00427AD4, 1);
    call_patch(get_string, 0x00427ADD, 7);
    memory_patch(0x00427BCD, mov_ecx);
    memory_patch(0x00427BCE, (u32)1546);
    call_patch(get_string, 0x00427BD2, 8);
     
    // getUnitRankString @ 42F370
    memory_patch(0x0042F3AF, mov_ecx);
    memory_patch(0x0042F3B0, (u32)572);
    jump_patch(get_string, 0x0042F3B4, 13);
    memory_patch(0x0042F3E8, mov_ecx);
    memory_patch(0x0042F3E9, (u32)571);
    jump_patch(get_string, 0x0042F3ED, 19);
    jump_patch(get_string, 0x0042F42A);
     
    // show_right_click_error_if_needed @ 455A00
    memory_patch(0x00455C3F, lea_ecx_eax1);
    call_patch(get_string, 0x00455C42, 8);
     
    // orders_PlaceScanner @ 464E40
    memory_patch(0x00464EAB, lea_ecx_eax1);
    call_patch(get_string, 0x00464EAE, 8);
     
    // do_targeted_order @ 46F5B0
    call_patch(get_string, 0x0046F8A0, 11);
    call_patch(get_string, 0x0046F987, 6);
    nop_patch(0x0046F995, 5);
     
    // insert_order_before @ 474540
    memory_patch(0x0047456D, mov_ecx);
    memory_patch(0x0047456E, (u32)872);
    call_patch(get_string, 0x00474572, 11);
     
    // perform_another_order @ 4745F0
    memory_patch(0x00474606, mov_ecx);
    memory_patch(0x00474607, (u32)872);
    call_patch(get_string, 0x0047460B, 11);
     
    // targeted_order @ 4754F0
    memory_patch(0x00475538, mov_ecx);
    memory_patch(0x00475539, (u32)871);
    call_patch(get_string, 0x0047553D, 1);
    nop_patch(0x00475546, 2);
    nop_patch(0x0047554B, 5);
    memory_patch(0x0047557C, mov_ecx);
    memory_patch(0x0047557D, (u32)871);
    call_patch(get_string, 0x00475581, 1);
    nop_patch(0x0047558A, 2);
    nop_patch(0x0047558F, 5);
     
    // getUnitNameStatString @ 47B090
    call_patch(get_string, 0x0047B0CB);
    memory_patch(0x0047B0D0, (u8)0xC3);//RETN
     
    // place_building_message @ 48D930
    call_patch(get_string, 0x0048D959, 11);
     
    // input_place_building_click_notify @ 48E4E0
    memory_patch(0x0048E509, mov_ecx);
    memory_patch(0x0048E50A, (u32)863);
    call_patch(get_string, 0x0048E50E, 8);
     
    // display_error_message_id_with_sfx @ 48EE30
    memory_patch(0x0048EEAB, lea_ecx_eax1);
    call_patch(get_string, 0x0048EEAE, 8);
     
    // playSfx_WorkerErrorBuildStart @ 48F6C0
    call_patch(get_string, 0x0048F760, 11);
     
    // playSfx_WorkerErrorBuildPlacement @ 48F7B0
    call_patch(get_string, 0x0048F850, 11);
     
    // spell_order @ 4926D0
    memory_patch(0x004927CD, (u8)0x41);//INC ECX
    call_patch(get_string, 0x004927CE, 10);
     
    // orders_Spell @ 492850
    memory_patch(0x004928DB, mov_ecx);
    memory_patch(0x004928DC, (u32)876);
    call_patch(get_string, 0x004928E0, 1);
    nop_patch(0x004928E9, 7);
     
    // orders_Recall @ 494470
    memory_patch(0x0049456D, lea_ecx_eax1);
    call_patch(get_string, 0x00494570, 8);
     
    // get_stat_txt_string @ 4C36F0
    jump_patch(get_string, 0x004C36F0, 1);
     
    // order_id_cases @ 4EBED0, perform_orders @ 4EC4D0   (orders_NukeLaunch @ 464730)
    memory_patch(0x004647B4, mov_ecx);
    memory_patch(0x004647B5, (u32)1047);
    call_patch(get_string, 0x004647B9, 3);
    nop_patch(0x004647C7, 5);
     
    // perform_orders @ 4EC4D0   (orders_InfestMine2 @ 4E98E0)
    memory_patch(0x004E996C, mov_ecx);
    memory_patch(0x004E996D, (u32)870);
    call_patch(get_string_edx, 0x004E9971, 1);
    nop_patch(0x004E997A, 2);
    nop_patch(0x004E997F, 5);
}

} // namespace hooks