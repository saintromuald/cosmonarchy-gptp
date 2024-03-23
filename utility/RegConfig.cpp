#include "RegConfig.h"

namespace RegConfig {

void overwrite_reg_config() {
	cpu_throttle.set(false);
}

HKEY open_or_create_starcraft_hkey() {
	HKEY hkey = 0;
	if (RegCreateKeyA(HKEY_CURRENT_USER, "SOFTWARE\\Blizzard Entertainment\\Starcraft", &hkey) != ERROR_SUCCESS) {
		return 0;
	}
	return hkey;
}

void RegDword::set(DWORD value) {
	HKEY hkey = open_or_create_starcraft_hkey();
	if (!hkey) return;
	
	RegSetKeyValueA(hkey, 0, m_key, REG_DWORD, &value, sizeof(value));
}

void RegString::set(std::string_view value) {
	HKEY hkey = open_or_create_starcraft_hkey();
	if (!hkey) return;

	RegSetKeyValueA(hkey, 0, m_key, REG_SZ, value.data(), value.size());
}

}
