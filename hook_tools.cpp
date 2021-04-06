#include "hook_tools.h"

//V241 for VS2008

//Injects a relative CALL to [target] at the [position].
//Original function from BWAPI by Kovarex; Modified by pastelmind
void callPatch(const void* target, void* position, const unsigned int nops) {
	uint8* const data = new uint8[5 + nops];
	data[0] = 0xE8; //Relative CALL instruction
	const uint32 address = (uint32)target - (uint32)position - 5;  //Relative address
	*(uint32*)(&data[1]) = address;
	for (unsigned int i = 0; i < nops; ++i)
		data[5 + i] = 0x90; //NOP instruction
	memoryPatch(position, data, 5 + nops);
	delete[] data;
}
void nops(const uint32 address, uint32 number) {
	uint8 nop = 0x90;
	for (int i = 0; i < number; i++) {
		memoryPatch((void*)(address + i), &nop, sizeof(uint8));
	}
}

//Injects a relative JMP to [target] at the [position].
//Original function from BWAPI by Kovarex; Modified by pastelmind
void jmpPatch(const void* target, void* position, unsigned int nops) {
	uint8* const data = new uint8[5 + nops];
	data[0] = 0xE9; //Relative JMP instruction
	const int32 address = (int32)target - (int32)position - 5;  //Relative address
	*(int32*)(&data[1]) = address;
	for (unsigned int i = 0; i < nops; ++i)
		data[5 + i] = 0x90; //NOP instruction
	memoryPatch(position, data, 5 + nops);
	delete[] data;
}

void wrapperPatch(const uint8* code, uint32 len, const uint32 address, unsigned int nop) {
	//function address
	auto exe = wrapperMemory.exec_alloc(len);
	if (nop > 0) {
		nops(address + 0x5, nop);
	}
	for (int i = 0; i < len; i++) {
		exe[i] = code[i];
	}
	uint32 diff = (uint32)exe - (address + 0x5);
	memoryPatch(address + 0x1, diff);
	memoryPatch(address, (uint8)0xe8);
}

//Inject an array of bytes, using the given length.
void memoryPatch(void* const address, const uint8* data, const size_t size) {
	DWORD oldProt = 0;
	VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProt);
	memcpy(address, data, size);
	VirtualProtect(address, size, oldProt, &oldProt);
}

ByteWrapperMemory wrapperMemory;
uint8* ByteWrapperMemory::exec_alloc(uint32 len) {
	std::shared_ptr<uint8[]> wrapper(new uint8[len]);
	wrapper_memory.push_back(wrapper);
	auto offset = &wrapper[0];
	return offset;
}