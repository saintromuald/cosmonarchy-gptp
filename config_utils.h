#pragma once

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

inline std::array<std::string, (size_t)PlayerColor::Count> make_player_color_identifiers() {
	std::array<std::string, (size_t)PlayerColor::Count> result;
	size_t i = 0;
	for (const char* str : player_color_names) {
		result[i++] = to_lower_snake_case(str);
	}
	return result;
}

inline const auto player_color_identifiers = make_player_color_identifiers();

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

template <typename T>
std::optional<T> yaml_try_cast(const YAML::Node& node) {
	try {
		return node.as<T>();
	} catch (const YAML::BadConversion&) {
		return std::nullopt;
	}
}

template <typename T>
YAML::Emitter& operator<<(YAML::Emitter& out, const T& data) {
	out << YAML::convert<T>::encode(data);
	return out;
}

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
		
		if (const auto string = yaml_try_cast<std::string>(node)) {
			const auto it = std::ranges::find(ui_bool_names, *string);
			if (it == ui_bool_names.end()) {
				return false;
			}

			value = EUiBool{it - ui_bool_names.begin()};
			return true;
		}

		return false;
	}
};

template <>
struct YAML::convert<PlayerColor> {
	static YAML::Node encode(PlayerColor value) {
		auto index = static_cast<size_t>(value);
		return YAML::Node{index < (size_t)PlayerColor::Count ? player_color_identifiers[index] : "invalid_color"};
	}

	static bool decode(const YAML::Node& node, PlayerColor& value) {
		if (!node) {
			return false;
		}

		if (const auto name = yaml_try_cast<std::string>(node)) {
			const auto it = std::ranges::find(player_color_identifiers, *name);
			if (it != player_color_identifiers.end()) {
				value = (PlayerColor)(it - player_color_identifiers.begin());
				return true;
			}
		}
		
		if (const auto index = yaml_try_cast<s32>(node)) {
			value = (PlayerColor)*index;
			return true;
		}

		return false;
	}
};
