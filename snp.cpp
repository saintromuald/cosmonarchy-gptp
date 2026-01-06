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
		m_crownlink_api.register_callback = reinterpret_cast<RegisterCallback>(GetProcAddress(crownlink, "register_status_callback"));
		m_crownlink_api.iterate_lobbies = reinterpret_cast<IterateLobbies>(GetProcAddress(crownlink, "iterate_lobbies"));
		m_crownlink_api.set_turns_per_second = reinterpret_cast<SetTurnsPerSecond>(GetProcAddress(crownlink, "set_turns_per_second"));
		m_crownlink_api.get_turns_per_second = reinterpret_cast<GetTurnsPerSecond>(GetProcAddress(crownlink, "get_turns_per_second"));
		m_crownlink_api.set_lobby_password = reinterpret_cast<SetLobbyPassword>(GetProcAddress(crownlink, "set_password"));
		m_crownlink_api.get_lobby_password = reinterpret_cast<GetLobbyPassword>(GetProcAddress(crownlink, "get_password"));
		m_crownlink_api.set_status_lobby = reinterpret_cast<SetStatusLobby>(GetProcAddress(crownlink, "set_status_lobby"));
		m_crownlink_api.set_map_name_edit = reinterpret_cast<SetMapNameEdit>(GetProcAddress(crownlink, "set_map_name_edit"));
		m_crownlink_api.get_status = reinterpret_cast<GetStatus>(GetProcAddress(crownlink, "get_status"));
		m_crownlink_api.get_version = reinterpret_cast<GetVersion>(GetProcAddress(crownlink, "version"));
	}
	m_provider_id = id;
	return true;
}

void ProviderManager::destroy_provider() {
	SNetDestroy();
	m_provider_id = 0;
	m_crownlink_api = {};
}

bool snp::ProviderManager::register_crownlink_status_callback(StatusCallback callback, void* user_data) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.register_callback) return false;
	m_crownlink_api.register_callback(callback, user_data);
	return true;
}

bool ProviderManager::iterate_lobbies(LobbyCallback callback, void* user_data) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.iterate_lobbies) return false;
	m_crownlink_api.iterate_lobbies(callback, user_data);
}

bool ProviderManager::set_turns_per_second(TurnsPerSecond turns_per_second) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.set_turns_per_second) return false;
	return m_crownlink_api.set_turns_per_second(turns_per_second);
}

bool ProviderManager::get_turns_per_second(TurnsPerSecond& output) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.get_turns_per_second) return false;
	output = m_crownlink_api.get_turns_per_second();
	return true;
}

bool ProviderManager::set_lobby_password(const char* password) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.set_lobby_password) return false;
	return m_crownlink_api.set_lobby_password(password);
}


bool ProviderManager::get_lobby_password(std::string& output) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.get_lobby_password) return false;
	char buffer[200];
	auto result = m_crownlink_api.get_lobby_password(buffer, sizeof(buffer));
	output = buffer;
	return true;
}

void ProviderManager::set_status_lobby(bool enable) {
	if (!m_provider_id) return;
	if (!is_crownlink()) return;
	if (m_crownlink_api.set_status_lobby) return;
	m_crownlink_api.set_status_lobby(enable);
}

void ProviderManager::set_map_name_edit(bool enable) {
	if (!m_provider_id) return;
	if (!is_crownlink()) return;
	if (!m_crownlink_api.set_map_name_edit) return;
	m_crownlink_api.set_map_name_edit(enable);
}

bool ProviderManager::get_crownlink_status(CrowServeState& output) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.get_status) return false;
	output = m_crownlink_api.get_status();
	return true;
}

bool ProviderManager::get_crownlink_version(u32 output) {
	if (!m_provider_id) return false;
	if (!is_crownlink()) return false;
	if (!m_crownlink_api.get_version) return false;
	output = m_crownlink_api.get_version();
	return true;
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