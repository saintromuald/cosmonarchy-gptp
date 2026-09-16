#pragma once
#include <utility>

namespace utils {
namespace detail {
template <typename Func>
class Defer {
public:
	[[nodiscard]] explicit Defer(Func func) : m_func(std::move(func)) {}
	~Defer() { m_func(); }

	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;

private:
	Func m_func;
};
} // namespace detail

template <typename Func>
[[nodiscard]] detail::Defer<Func> defer(Func func) {
	return detail::Defer<Func>(std::move(func));
}
} // namespace utils
