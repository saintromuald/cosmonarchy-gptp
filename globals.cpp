#include "globals.h"
#include "hook_tools.h"
globals Globals;

SaveState globals::aise_save() {
	static SaveState state;
	output.clear();
	std::ostringstream stream(std::stringstream::out|std::stringstream::binary);
	stream.write(reinterpret_cast<const char*>(&example_placeholder), sizeof(example_placeholder));
	output = stream.rdbuf()->str();
	if (output.length() == 0) {
		return state;
		//error
	}
	state.ptr = output.c_str();
	state.len = output.size();
//	char buff[64];
//	sprintf(buff, "State: %d %d, bytes: %d %d %d %d", state.ptr, state.len, buf[0],buf[1],buf[2],buf[3]);
//	scbw::printText(buff);
	return state;
}
void globals::aise_load(const char* ptr, u32 len){
	if (len == 0 || ptr==0) {
		return;//error
	}
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	std::string str(ptr, len);
	stream << str;
	stream.read(reinterpret_cast<char*>(&example_placeholder), sizeof(example_placeholder));
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
