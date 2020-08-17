#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <map>

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

// Saving
template <typename T> // 1-Dimensional Array
std::ostream& operator<<(std::ostream& stream, T arr[]) {
	for (int i = 0; i < LENGTH(arr); i++) {
		stream << arr[i] << "\0";
	}
	stream << "\0";
	return stream;
}

template <typename T> // 2-Dimensional Array
std::ostream& operator<<(std::ostream& stream, T arr[][]) {
	for (int i = 0; i < LENGTH(arr); i++) {
		for (int j = 0; j < LENGTH(arr[i]); j++) {
			stream << arr[i][j] << "\0";
		}
		stream << "\0";
	}
	stream << "\0";
	return stream;
}

template <typename T, typename Y> // Map
std::ostream& operator<<(std::ostream& stream, std::map<T, Y>& map) {
	for (std::pair<T, Y>& pair : map) {
		stream << pair.first << "\0" << pair.second << "\0"
	}
	stream << "\0";
	return stream;
}

template <typename T> // Vector
std::ostream& operator<<(std::ostream& stream, std::vector<T>& vec) {
	for (T& value : vec) {
		stream << value << "\0";
	}
	stream << "\0";
	return stream;
}

template <typename T> // Undefined Object
std::ostream& operator<<(std::ostream& stream, T& object) {
	stream << *(char*)&object << "\0";
	return stream;
}

// Loading
template <typename T> // 1-Dimensional Array
std::istream& operator>>(std::istream& stream, T const* arr) {
	std::string str;
	int i = 0;
	do {
		stream >> str;
		arr[i++] = *(T*)&str;
	} while (str != "");
	return stream;
}

template <typename T> // 2-Dimensional Array
std::istream& operator>>(std::istream& stream, T const** arr) {
	std::string str;
	int i = 0;
	int j = 0;
	do {
		do {
			stream >> str;
			arr[i][j++] = *(T*)&str;
		} while (str != "");
		i++;
	} while (str != "");
	return stream;
}

template <typename T, typename Y> // Map
std::istream& operator>>(std::istream& stream, std::map<T, Y>& map) {
	std::string key;
	std::string value;
	do {
		stream >> key >> value;
		map.put(*(T*)&key, *(Y*)&value)
	} while (str != "");
	return stream;
}

template <typename T, typename Y> // Vector
std::istream& operator>>(std::istream& stream, std::vector<T, Y>& vec) {
	std::string str;
	do {
		stream >> str;
		vec.push_back(*(T*)&str);
	} while (str != "");
	return stream;
}

template <typename T> // Undefined Object
std::istream& operator>>(std::istream& stream, T& object) {
	stream >> *(char*)&object;
	return stream;
}

#undef LENGTH