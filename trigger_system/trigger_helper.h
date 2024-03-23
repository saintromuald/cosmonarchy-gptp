#pragma once

template <typename T>
class LazyInit {
public:
	LazyInit() { data = new T(); }
	~LazyInit() { delete data; }

	LazyInit(const LazyInit&) = delete;
	LazyInit& operator=(const LazyInit&) = delete;

	void reset() {
		delete data;
		data = new T();
	}

	T* operator->() {
		return data;
	}

private:
	T* data;
};

