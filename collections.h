#pragma once
#include <algorithm>
#include <ranges>

// Currently only supports Plain Old Data (POD), Destructors do not function here
template <typename T, size_t N>
class SmallArray {
public:
	inline size_t size() { return m_size; }
	inline size_t capacity() { return N; }

	SmallArray() { clear(); }

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
	size_t m_size = {};
	T m_data[N] = {};
};