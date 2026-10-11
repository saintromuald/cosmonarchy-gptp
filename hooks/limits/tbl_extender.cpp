#include "tbl_extender.h"
#include "storm/storm.h"
#include <fstream>
#include <string>
#include <format>

namespace hooks {

static void collapse_control_codes(std::string& line) {
    for (size_t i = 0; i < line.size(); i++) {
        switch (line[i]) {
            case '<': {
                for (size_t j = i + 1; j < line.size(); j++) {
                    if (line[j] == '>') {
                        u32 control_char = 0;
                        const auto first = line.c_str() + i + 1;
                        const auto last = line.c_str() + j;
                        const auto result = std::from_chars(first, last, control_char, 10);
                        if (result.ec == std::errc{} && result.ptr == last) {
                            line[i] = char(control_char);
                            line.erase(i + 1, j - i);
                        }
                        break;
                    }
                }
            } break;
            case '\\': {
                // Safe to check i+1, since we have the ending null char as buffer
                switch (line[i+1]) {
                    case 'n': {
                        line[i] = '\n';
                        line.erase(i + 1, 1);
                    } break;
                    case '\\': {
                        line.erase(i + 1, 1);
                    } break;
                }
            } break;
        }
    }
}

template <typename... Ts>
static void load_stat_txt_error(const std::format_string<Ts...>& format, Ts&&... args) {
    //const auto msg = std::vformat(format, std::make_format_args(std::forward<Ts>(args)...));
    const auto msg = std::format(format, std::forward<Ts>(args)...);
    MessageBoxA(0, msg.c_str(), "Load Stat Txt Error", MB_OK | MB_ICONHAND);
    exit(-1);
}

void __cdecl load_stat_txt() { // 004C3A20
    static constexpr auto RezStatTxt = "rez\\stat.txt";
    if (!mpq_file_exists(RezStatTxt)) {
        load_stat_txt_error("File {} not found", RezStatTxt);
    }
    std::istringstream stream{mpq_load_file(RezStatTxt).value_or("")};

    size_t data_size = 0;
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        collapse_control_codes(line);
        data_size += line.size() + 1; // + null char
        lines.push_back(std::move(line));
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
     
    // drawCancelUpgradeContextHelp @ 4575A0
    memory_patch(0x004575BE, mov_ecx);
    memory_patch(0x004575BF, (u32)805);
    call_patch(get_string, 0x004575C3, 11);
    memory_patch(0x004575FC, mov_ecx);
    memory_patch(0x004575FD, (u32)806);
    call_patch(get_string, 0x00457601, 11);
    memory_patch(0x0045762E, (u8)0x48);//Change EDX to ECX
    memory_patch(0x00457636, (u8)0x41);//INC ECX
    call_patch(get_string, 0x00457637, 4);
     
    // statdata_user_interact @ 4576A0
    memory_patch(0x004576EC, mov_ecx);
    memory_patch(0x004576ED, (u32)777);
    call_patch(get_string, 0x004576F1);
    memory_patch(0x004576F6, mov_edi_eax);
    nop_patch(0x004576F8, 6);
    memory_patch(0x00457739, mov_ecx);
    memory_patch(0x0045773A, (u32)1301);
    call_patch(get_string, 0x0045773E);
    memory_patch(0x00457743, mov_ecx_eax);
    nop_patch(0x00457745, 9);
    memory_patch(0x00457759, mov_ecx);
    memory_patch(0x0045775A, (u32)1301);
    call_patch(get_string, 0x0045775E);
    memory_patch(0x00457763, mov_edi_eax);
    memory_patch(0x004577B1, mov_ecx);
    memory_patch(0x004577B2, (u32)1301);
    call_patch(get_string, 0x004577B6, 11);
    memory_patch(0x004577D1, mov_ecx);
    memory_patch(0x004577D2, (u32)777);
    call_patch(get_string, 0x004577D6);
    memory_patch(0x004577DB, mov_edi_eax);
    memory_patch(0x00457834, mov_ecx);
    memory_patch(0x00457835, (u32)778);
    call_patch(get_string, 0x00457839);
    memory_patch(0x0045783E, mov_ecx_eax);
    nop_patch(0x00457840, 6);
    memory_patch(0x00457899, mov_edx);
    memory_patch(0x0045789A, (u32)779);
    call_patch(get_string_edx, 0x0045789E, 8);
    memory_patch(0x0045792E, mov_ecx);
    memory_patch(0x0045792F, (u32)777);
    call_patch(get_string, 0x00457933);
    memory_patch(0x00457938, mov_ebx_eax);
    nop_patch(0x0045793A, 9);
    memory_patch(0x0045794E, mov_ecx);
    memory_patch(0x0045794F, (u32)791);
    call_patch(get_string, 0x00457953);
    memory_patch(0x00457958, mov_edi_eax);
    memory_patch(0x0045798D, mov_ecx);
    memory_patch(0x0045798E, (u32)777);
    call_patch(get_string, 0x00457992);
    memory_patch(0x00457997, mov_ebx_eax);
    nop_patch(0x00457999, 9);
    memory_patch(0x004579AD, mov_ecx);
    memory_patch(0x004579AE, (u32)792);
    call_patch(get_string, 0x004579B2);
    memory_patch(0x004579B7, mov_edi_eax);
    memory_patch(0x004579D0, mov_ecx);
    memory_patch(0x004579D1, (u32)793);
    call_patch(get_string, 0x004579D5);
    memory_patch(0x004579DA, mov_ecx_eax);
    nop_patch(0x004579DC, 6);
    memory_patch(0x00457A25, mov_ecx);
    memory_patch(0x00457A26, (u32)777);
    call_patch(get_string, 0x00457A2A);
    memory_patch(0x00457A2F, mov_ebx_eax);
    nop_patch(0x00457A31, 9);
    memory_patch(0x00457A45, mov_ecx);
    memory_patch(0x00457A46, (u32)794);
    call_patch(get_string, 0x00457A4A);
    memory_patch(0x00457A4F, mov_edi_eax);
     
    // drawCancelUnitContextHelp @ 457B10
    memory_patch(0x00457BAB, lea_ecx_eax1);
    call_patch(get_string, 0x00457BAE);
    memory_patch(0x00457BB3, mov_esi_eax);
    nop_patch(0x00457BB5, 6);
     
    // drawUnloadUnitContextHelp @ 457C20
    memory_patch(0x00457C29, mov_ecx);
    memory_patch(0x00457C2A, (u32)808);
    call_patch(get_string, 0x00457C2E);
    memory_patch(0x00457C38, mov_esi_eax);
    nop_patch(0x00457C3A, 5);
     
    // statdataMouseOverInteract @ 457CE0 //Handled by selection extender jump_patch @ 457D87
    /*
    memory_patch(0x00457D81, mov_ecx);
    memory_patch(0x00457D82, (u32)809);
    call_patch(get_string, 0x00457D86, 8);
    */
     
    // create_upgrade_level_tooltip @ 459150
    memory_patch(0x0045916A, mov_edx);
    memory_patch(0x0045916B, (u32)795);
    call_patch(get_string_edx, 0x0045916F, 8);
     
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
     
    // drawShowHideTerrainContextHelp @ 4A4C40
    memory_patch(0x004A4C50, mov_edx);
    memory_patch(0x004A4C51, (u32)810);
    call_patch(get_string_edx, 0x004A4C55, 2);
    memory_patch(0x004A4C67, mov_edx);
    memory_patch(0x004A4C68, (u32)811);
    call_patch(get_string_edx, 0x004A4C6C, 6);
     
    // drawStatLBBtnsContextHelp @ 4A5110
    memory_patch(0x004A5139, mov_edx);
    memory_patch(0x004A513A, (u32)1540);
    call_patch(get_string_edx, 0x004A513E, 2);
    memory_patch(0x004A5150, mov_edx);
    memory_patch(0x004A5151, (u32)812);
    call_patch(get_string_edx, 0x004A5155, 6);
    memory_patch(0x004A5182, mov_edx);
    memory_patch(0x004A5183, (u32)1540);
    call_patch(get_string_edx, 0x004A5187, 8);
     
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