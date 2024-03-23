#pragma once
#include <BW/api.h>

namespace RegConfig {

void overwrite_reg_config();
HKEY open_or_create_starcraft_hkey();

class RegDword {
public:
	RegDword(const char* key) : m_key(key) {}

	void set(DWORD value);

private:
	const char* m_key;
};

class RegString {
public:
	RegString(const char* key) : m_key(key) {}

	void set(std::string_view value);

private:
	const char* m_key;
};

class RegMultiString {
public:
	RegMultiString(const char* key) : m_key(key) {}

	template <typename T>
	void set(const T& strings);

private:
	const char* m_key;
};

template <typename T>
void push_chars(std::vector<char>& vec, T const& str) {
	for (char c : str) {
		vec.push_back(c);
	}
}

template <>
void push_chars(std::vector<char>& vec, const char* const& str) {
	for (const char* c = str; *c; c++) {
		char ch = *c;
		vec.push_back(ch);
	}
}

template <typename T>
void RegMultiString::set(const T& strings) {
	HKEY hkey = open_or_create_starcraft_hkey();
	if (!hkey) return;
	
	std::vector<char> vec;
	for (auto& string : strings) {
		push_chars(vec, string);
		vec.push_back(0);
	}
	vec.push_back(0);
	
	RegSetKeyValueA(hkey, 0, m_key, REG_MULTI_SZ, vec.data(), vec.size());
}

inline RegDword bldgnoise("bldgnoise");
inline RegDword color_cycle("ColorCycle");
inline RegDword cpu_throttle("CPUThrottle");
inline RegDword firewall_check_done("FirewallCheckDone");
inline RegDword gamma("Gamma");
inline RegDword intro("intro");
inline RegDword intro_x("introX");
inline RegDword kscroll("kscroll");
inline RegDword mscroll("mscroll");
inline RegDword m_kscroll("m_kscroll");
inline RegDword m_mscroll("m_mscroll");
inline RegDword music("music");
inline RegDword sfx("sfx");
inline RegDword speed("speed");
inline RegDword tip("tip");
inline RegDword tip_num("tipnum");
inline RegDword trig_text("trigtext");
inline RegDword unit_noise("unitnoise");
inline RegDword unit_portraits("UnitPortraits");
inline RegDword unit_speech("unitspeech");
inline RegDword use_chat_colors("usechatcolors");
inline RegDword os_compatibility_check_done("OsCompatibilityModeCheckDone");
inline RegDword prefer_port_6112("PreferPort6112");

inline RegString custom_type("Custom Type");
inline RegString game_speed("Game Speed");
inline RegString game_subtype("Game Subtype");
inline RegString game_type("Game Type");
inline RegString last_created_game_name("Last Created Game Name");

inline RegMultiString recent_maps("Recent Maps");

};
