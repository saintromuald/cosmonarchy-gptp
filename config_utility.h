#pragma once
#include "yaml-cpp/yaml.h"

template<typename T>
void LoadField(const YAML::Node& node, const std::string& key, T& variable)
{
	if (node[key])
	{
		variable = node[key].as<T>();
	}
}

template<typename T>
void write_field(YAML::Emitter& emitter, const std::string& key, const T& value)
{
	emitter << YAML::Key << key << YAML::Value << value;
}