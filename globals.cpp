#include "globals.h"
#include "hook_tools.h"
globals Globals;

SaveState globals::aise_save() {
	static SaveState state;
	output.clear();
	std::ostringstream stream(std::stringstream::out|std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.write(reinterpret_cast<const char*>(&extended_sfx), sizeof(extended_sfx));
	stream.write(reinterpret_cast<const char*>(&tbl_extender), sizeof(tbl_extender));
	stream.write(reinterpret_cast<const char*>(&unitid_extender), sizeof(unitid_extender));
	output = stream.rdbuf()->str();
	if (output.length() == 0) {
		return state;
		//error
	}
	state.ptr = output.c_str();
	state.len = output.size();
	return state;
}
void globals::aise_load(const char* ptr, u32 len){
	if (len == 0 || ptr==0) {
		return;//error
	}
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
	stream.read(reinterpret_cast<char*>(&extended_unit_table), sizeof(extended_unit_table));
	stream.read(reinterpret_cast<char*>(&extended_sfx), sizeof(extended_sfx));
	stream.read(reinterpret_cast<char*>(&tbl_extender), sizeof(tbl_extender));
	stream.read(reinterpret_cast<char*>(&unitid_extender), sizeof(unitid_extender));

}
void __declspec(naked) Globals_SaveStateWrapper() {
	static SaveState* state;
	__asm {
		PUSHAD
	}
	state = &Globals.aise_save();
	__asm {
		POPAD
		MOV EAX, state
		RETN
	}
}

void __declspec(naked) Globals_LoadStateWrapper() {
	static const char* pointer;
	static u32 len;
	__asm {
		MOV len,ECX
		MOV pointer,EAX
		PUSHAD
	}
	Globals.aise_load(pointer, len);
	__asm {
		POPAD
		RETN
	}
}

void injectGlobalsHooks() {
	jmpPatch(Globals_SaveStateWrapper, 0x004AAF23);
	jmpPatch(Globals_LoadStateWrapper, 0x004AAE94);
}
