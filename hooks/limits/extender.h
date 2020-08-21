#pragma once
#include <vector>
#include <sstream>
#include "SCBW\api.h"
#include "hook_tools.h"
#include "tbl_extender.h"
#define NO_IDENTIFIER -1
//
const int SFX_Entries = 5;
//
const int SFX_Default_Length = 1144;

std::vector<std::string> split_by(std::string line, char delimiter);
bool starts_with(std::string line, char c);
std::string to_lowercase(std::string line);

static inline void ltrim(std::string& s);
static inline void rtrim(std::string& s);
static inline void trim(std::string& s);
bool last_char(std::string s, char c);
class Extender {
	virtual std::vector<u32> get_address_list() = 0;
	virtual std::vector<u32> get_size_list() = 0;
public:
	virtual void extend() = 0;
	virtual int identifier(std::string str) = 0;
	void uplink(std::vector<u32> address_list, u32 address);
	int get_file_entry_count(char* name, u32 entries);
	void parse_set(char* name, u32 default_max_limit, u32 entries);
	virtual u32 parse_entry_value(std::string str,int id) = 0;
	virtual bool is_string_id(int id) = 0;
	virtual void set_value(int entry, int id, std::string str) = 0;
	virtual void set_value(int entry, int id, int str) = 0;
	virtual void set_extended_data() = 0;
};


namespace hooks {
	void injectLimitHooks();
}
namespace Reg {
	enum {
		_EAX = 1,
		_EBX,
		_ECX,
		_EDX,
		_ESI,
		_EDI,

		L_AX,
		L_BX,
		L_CX,
		L_DX,
		L_SI,
		L_DI,
		__NOP,
	};
}
namespace TblPatchType {
	enum {
		CmpPatch = 1,
		PtrWriting,
		PtrMathWriting,
		RegisterPointerCmp,
		RegisterPointerMov,
		RegisterCmp,
		RegisterWriting,
		NopPatch,
		AddPatch,
		//DecPatch,
		JmpPatch,


	};
}
namespace Jmp {
	enum {
		JA,
		JBE,
	};
}