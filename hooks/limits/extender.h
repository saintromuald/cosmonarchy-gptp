#pragma once
#include <vector>
#include <sstream>
#include "SCBW\api.h"
#include "hook_tools.h"
#include "tbl_extender.h"
#define NO_IDENTIFIER -1
//
const s32 SFX_Entries = 5;
//
const s32 SFX_Default_Length = 1144;

std::vector<std::string> split_by(std::string line, char delimiter);
bool starts_with(std::string line, char c);
std::string to_lowercase(std::string line);

static inline void ltrim(std::string& s);
static inline void rtrim(std::string& s);
static inline void trim(std::string& s);
bool last_char(std::string s, char c);
struct Extender
{
	virtual std::vector<u32> get_address_list() = 0;
	virtual std::vector<u32> get_size_list() = 0;
	virtual void extend() = 0;
	virtual s32 identifier(std::string str) = 0;
	void uplink(std::vector<u32> address_list, u32 address);
	s32 get_file_entry_count(char* name, u32 entries);
	void parse_set(char* name, u32 default_max_limit, u32 entries);
	virtual u32 parse_entry_value(std::string str, s32 id) = 0;
	virtual bool is_string_id(s32 id) = 0;
	virtual void set_value(s32 entry, s32 id, std::string str) = 0;
	virtual void set_value(s32 entry, s32 id, s32 str) = 0;
	virtual void set_extended_data() = 0;
};


namespace hooks
{
	void inject_limit_hooks();
}
namespace Reg
{
	enum
	{
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
namespace TblPatchType
{
	enum
	{
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
namespace Jmp
{
	enum
	{
		JA,
		JBE,
	};
}

static u32 fakeExtFlingyIds[228];