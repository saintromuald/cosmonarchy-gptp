#pragma once
#include <algorithm>
#include <ranges>

#include "types.h"

// Currently only supports Plain Old Data (POD), Destructors do not function here
template <typename T, size_t N>
class StaticArray {
public:
	inline size_t size() { return m_size; }
	inline size_t capacity() { return N; }

	StaticArray() { clear(); }

	void append(T elem) {
		if (m_size >= N) {
			return;
		}
		m_data[m_size++] = elem;
	}

	T pop() {
		if (m_size <= 0) {
			return {};
		}
		return m_data[--m_size];
	}

	T remove(size_t index) {
		if (index >= m_size) {
			return {};
		}
		T result = m_data[index];
		m_data[index] = m_data[--m_size];
		return result;
	}

	void resize(size_t new_size) {
		if (new_size > N) {
			new_size = N;
		}
		for (size_t i = m_size; i < new_size; i++) {
			m_data[i] = {};
		}
		m_size = new_size;
	}

	void clear() {
		m_size = 0;
		memset(m_data, 0, sizeof(m_data));
	}

	T& operator[](size_t index) {
		return m_data[index];
	}

	T* begin() { return m_data; }
	T* end() { return m_data + m_size; }

private:
	size_t m_size = 0;
	T m_data[N] = {};
};

template <typename T>
class Array {
public:
	inline size_t size() { return m_size; }
	inline size_t capacity() { return m_capacity; }

	Array() : m_size(0), m_capacity(0), m_data(0) {}
	~Array() { clear(); }
	Array(const Array&) = delete;
	Array& operator=(const Array&) = delete;

	void reserve(size_t n) {
		if (m_size + n <= m_capacity) {
			return;
		}
		m_capacity = (m_capacity + n) * 2;
		if (m_capacity < 8) m_capacity = 8;
		m_data = realloc(m_data, m_capacity);
	}

	void append(T elem) {
		reserve(1);
		m_data[m_size++] = elem;
	}

	T pop() {
		if (m_size <= 0) {
			return {};
		}
		return m_data[--m_size];
	}

	T remove(size_t index) {
		if (index >= m_size) {
			return {};
		}
		T result = m_data[index];
		m_data[index] = m_data[--m_size];
		return result;
	}

	void resize(size_t new_size) {
		if (new_size > m_capacity) {
			reserve(new_size - m_capacity);
			for (size_t i = m_size; i < new_size; i++) {
				m_data[i] = {};
			}
		}
		m_size = new_size;
	}

	void clear() {
		m_size = 0;
		m_capacity = 0;
		if (m_data) {
			free(m_data);
			m_data = 0;
		}
	}

	T& operator[](size_t index) {
		return m_data[index];
	}

	T* begin() { return m_data; }
	T* end() { return m_data + m_size; }

private:
	size_t m_size;
	size_t m_capacity;
	T* m_data;
};