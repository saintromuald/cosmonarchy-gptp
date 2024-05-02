#pragma once

enum class EUiBool {
	False, InGameOnly, WitnessOnly, True, Count
};

constexpr std::array<const char*, (size_t)EUiBool::Count> ui_bool_names { "false", "ingame", "witness", "true" };

struct UiBool {
	EUiBool value = EUiBool::False;

	UiBool() = default;
	UiBool(bool value) : value(value ? EUiBool::True : EUiBool::False) {}
	UiBool(EUiBool value) : value(value) {}

	operator bool() const {
		switch (value) {
			case EUiBool::False: return false;
			case EUiBool::InGameOnly: return !bw::in_replay_or_witness();
			case EUiBool::WitnessOnly: return bw::in_replay_or_witness();
			case EUiBool::True: return true;
		}
	}
};

template <>
struct YAML::convert<UiBool> {
	static YAML::Node encode(const UiBool& value) {
		auto index = static_cast<size_t>(value.value);
		return YAML::Node{ui_bool_names[index < ui_bool_names.size() ? index : 0]};
	}

	static bool decode(const YAML::Node& node, UiBool& value) {
		if (!node) {
			return false;
		}

		const auto it = std::ranges::find(ui_bool_names, node.as<std::string>());
		if (it == ui_bool_names.end()) {
			return false;
		}

		const auto index = it - ui_bool_names.begin();
		value = EUiBool{index};
		return true;
	}
};

inline std::string to_lower_snake_case(const std::string& str) {
	std::string result;
	result.reserve(str.size());
	for (char c : str) {
		if (isspace(c)) {
			result += '_';
		} else if (isalnum(c)) {
			result += tolower(c);
		}
	}
	return result;
}

inline std::array<std::string, PlayerColor::Count> make_player_color_identifiers() {
	std::array<std::string, PlayerColor::Count> result;
	size_t i = 0;
	for (const char* str : player_color_names) {
		result[i++] = to_lower_snake_case(str);
	}
	return result;
}

inline auto player_color_identifiers = make_player_color_identifiers();

template <>
struct YAML::convert<PlayerColor::Enum> {
	static YAML::Node encode(PlayerColor::Enum value) {
		auto index = static_cast<size_t>(value);
		return YAML::Node{index < PlayerColor::Count ? player_color_identifiers[index] : "invalid_color"};
	}

	static bool decode(const YAML::Node& node, PlayerColor::Enum& value) {
		if (!node) {
			return false;
		}

		const auto it = std::ranges::find(player_color_identifiers, node.as<std::string>());
		if (it == player_color_identifiers.end()) {
			return false;
		}

		const auto index = it - player_color_identifiers.begin();
		value = (PlayerColor::Enum)index;
		return true;
	}
};
