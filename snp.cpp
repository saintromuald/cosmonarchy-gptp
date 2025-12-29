#include "snp.h"
#include "snp.h"

namespace snp {
std::vector<SPIProvider>& snp::ProviderManager::available_providers() {
	m_providers.clear();
	SNetEnumProviders(0, add_provider_to_list);
	return m_providers;
}

bool ProviderManager::is_crownlink() {
	if (!g_spi_provider) return false;
	return g_provider_id == 'CNLK' || g_provider_id == 'CLDB';
}

bool ProviderManager::initialize_provider(u32 id) {
	auto result = helper::initialize_spi(id);
	if (!result) return false;
	if (id == 'CNLK' || id == 'CLDB') {
		auto crownlink = GetModuleHandleA("CrownLink.snp");
		if (!crownlink) {
			SNetDestroy();
			return false;
		}
		m_register_callback = reinterpret_cast<RegisterCallback>(GetProcAddress(crownlink, "RegisterStatusCallback"));
		m_set_turns_per_second = reinterpret_cast<SetTurnsPerSecond>(GetProcAddress(crownlink, "SetTurnsPerSecond"));
	}
	m_provider_id = id;
	return true;
}

void ProviderManager::destroy_provider() {
	SNetDestroy();
	m_provider_id = 0;
	m_register_callback = {};
}

bool snp::ProviderManager::register_crownlink_status_callback(StatusCallback callback, bool enable_status_lobby, bool edit_name) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_register_callback) return false;
	m_register_callback(callback, enable_status_lobby, edit_name);
	return true;
}

bool ProviderManager::set_turns_per_second(u32 turns_per_second) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_set_turns_per_second) return false;
	return m_set_turns_per_second(turns_per_second);
}

b32 __stdcall snp::ProviderManager::add_provider_to_list(u32 id, const char* name, const char* description, Caps* caps) {
	auto& provider_manager = ProviderManager::instance();
	SPIProvider provider{};
	provider.id = id;
	safe_copy(provider.name, name);
	safe_copy(provider.description, name);
	std::memcpy(&provider.caps, caps, sizeof(caps));
	provider_manager.providers().push_back(std::move(provider));
	return true;
}
} // namespace snp