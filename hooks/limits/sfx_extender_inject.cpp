#include "hooks/limits/sfx_extender.h"

std::vector<u32> SFXExtender::get_address_list(){
	return {};
}
u32 SFXExtender::parse_entry_value(std::string str, int id) {
    return std::stoi(str);
}
int SFXExtender::identifier(std::string str) {
    if (str == "sound_file") {
        return SfxDatEntries::SoundFilePtrs;
    }
    else if (str == "volume" || str == "min_volume") {
        return SfxDatEntries::MinVolume;
    }
    else if (str == "flags") {
        return SfxDatEntries::Flags;
    }
    else if (str == "priority" || str == "unknown1") {
        return SfxDatEntries::Priority;
    }
    else if (str == "race") {
        return SfxDatEntries::Race;
    }
    return NO_IDENTIFIER;
}

void SFXExtender::set_value(int entry, int id, std::string str) {
    
}
void SFXExtender::set_value(int entry, int id, int val) {
    switch (val) {
    case SfxDatEntries::MinVolume:
        min_volume[entry] = val;
        break;
    case SfxDatEntries::Flags:
        flags[entry] = val;
        break;
    case SfxDatEntries::Priority:
        priority[entry] = val;
    case SfxDatEntries::Race:
        race[entry] = val;
    default:
        break;
    }
}
std::vector<u32> SFXExtender::get_size_list(){
	return {
        0x0045F042,
        0x004BBC2D,
        0x004BCA98,
        0x004BCD2A,
        0x004DB0FA,
        0x005154A0,
        0x005154AC,
        0x005154B8,
        0x005154C4,
        0x005154D0,
    };
}

std::vector<u32> SFXExtender::get_flags_list() {
    return {
0x004BB972,
0x004BBC54,
0x004BBC67,
0x004BBF17,
0x004BBF20,
0x004BBFF3,
0x004BC06C,
0x004BC0C0,
0x004BC7AE,
0x004BC80A,
0x004BC846,
0x004BCAD6,
0x004BCAF3,
0x004BCB0B,
0x004BCD44,
//0x005154B0,
    };
}

bool SFXExtender::is_string_id(int id) {
    if (id == SfxDatEntries::SoundFilePtrs) {
        return true;
    }
    return false;
}

void SFXExtender::extend() {
    
    int extra_entries = get_file_entry_count("samase\\sfx_ext.txt", SFX_Entries);
	if (extra_entries>0) {
        int new_count = extra_entries + SOUND_TYPE_COUNT;
        unk.resize(new_count);
        ptr = &unk[0];
        race.resize(new_count);
        min_volume.resize(new_count);
        flags.resize(new_count);
        sound_file_ptrs.resize(new_count);
        uplink(get_size_list(), new_count);
        memoryPatch(0x004BBCA5, new_count - 1);
        memoryPatch(0x004BBEB7, new_count - 1);
        memoryPatch(0x0046102C+1, &sound_file_ptrs[0] + 0x58);
        memoryPatch(0x0046116C+1, &sound_file_ptrs[0] + 0x54);
        memoryPatch(0x004BC68D+3, &sound_file_ptrs[0]);
        memoryPatch(0x004BCA39+3, &sound_file_ptrs[0]);
        memoryPatch(0x004BCB29+1, &sound_file_ptrs[0] + 0x3C);
        memoryPatch(0x004DB0E5+1, &sound_file_ptrs[0]);
        memoryPatch(0x004BC5E9+3, &sound_file_ptrs[0]);//
        memoryPatch(0x004BCA04+1, &sound_file_ptrs[0]);//
        //memoryPatch(0x00515498, &sound_file_ptrs[0]);
        memoryPatch(0x00455BD9+3, &min_volume[0]);
        memoryPatch(0x0048D0ED+3, &min_volume[0] + 0x17);
        memoryPatch(0x0048D169+3, &min_volume[0] + 2);
        memoryPatch(0x0048D406+3, &min_volume[0] + 0x17);
        memoryPatch(0x0048ECEA+3, &min_volume[0]);
        memoryPatch(0x0048EDD4+3, &min_volume[0]);
        memoryPatch(0x0048EE59+3, &min_volume[0]);
        memoryPatch(0x0048EFA2+3, &min_volume[0]);
        memoryPatch(0x0048F025+3, &min_volume[0]);
        memoryPatch(0x0048F0C7+3, &min_volume[0]);
        memoryPatch(0x0048F1A4+3, &min_volume[0]);
        memoryPatch(0x004E3EB2+3, &min_volume[0]);
        memoryPatch(0x004E3F52+3, &min_volume[0]);
        //memoryPatch(0x005154C8, &min_volume[0]);
        uplink(get_flags_list(), (u32)&flags[0]);
        memoryPatch(0x0048F123+4, &race[0]);//unk3 in farty's list
        memoryPatch(0x0048F200+4, &race[0]);
        memoryPatch(0x0048F2EB, &race[0]);
        memoryPatch(0x0048F376, ((u32)&race[0]) + 0x10E);
        memoryPatch(0x0048F3D2, &race[0]);
        memoryPatch(0x0048F41B, ((u32)&race[0]) + 0x10E);
        memoryPatch(0x0048F4A0, &race[0]);
        memoryPatch(0x0048F572, &race[0]);
        memoryPatch(0x0048F603+4, &race[0]);
        memoryPatch(0x0048F697, &race[0]);
        memoryPatch(0x0048F732, &race[0]);
        memoryPatch(0x0048F822, &race[0]);
        memoryPatch(0x0048F8E3, ((u32)&race[0]) + 0x104);
        memoryPatch(0x0048FAE7+1, &race[0]);
        //memoryPatch(0x005154BC, &race[0]);
        memoryPatch(0x004BBEC3, &priority[0]);
        memoryPatch(0x004BC0D3, &priority[0]);
        memoryPatch(0x004BC813, &priority[0]);
        memoryPatch(0x005154A4, &priority[0]);
        auto snd_pointer = (u32)this->ptr;
        memoryPatch(0x004BB8E8, snd_pointer);
        memoryPatch(0x004BBC47, snd_pointer);
        memoryPatch(0x004BC077, snd_pointer);
        memoryPatch(0x004BC674, snd_pointer);
        memoryPatch(0x004BCAA6, snd_pointer);
        memoryPatch(0x0045EF66, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F11D, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F1FA, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F2DE, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F3CA, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F498, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F56A, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F5FF, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F68F, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F72A, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F81A, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048FAE3, ((u32)snd_pointer) + 8);
        memoryPatch(0x0048F8DA, ((u32)snd_pointer) + 0x828);
        memoryPatch(0x0048F36F, ((u32)snd_pointer) + 0x878);
        memoryPatch(0x0048F412, ((u32)snd_pointer) + 0x878);
	}
}

namespace hooks {
	void sfxExtender() {
		new_sfxdata.extend();
	}
}

SFXExtender new_sfxdata;