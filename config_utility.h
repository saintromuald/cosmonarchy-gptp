#pragma once
#include "yaml-cpp/yaml.h"

template<typename T>
void LoadField(const YAML::Node& node, T& variable, const std::string& key) {
	if (node[key]) {
		variable = node[key].as<T>();
	}
}

template<typename T>
void WriteField(YAML::Emitter& emitter, const std::string& key, const T& value) {
	emitter << YAML::Key << key << YAML::Value << value;
}