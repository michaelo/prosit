#pragma once

// https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/ (thanks!)
template <typename F>
struct _privDefer {
	F f;
	_privDefer(F f) : f(f) {}
	~_privDefer() { f(); }
};

template <typename F>
_privDefer<F> _defer_func(F f) {
	return _privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = _defer_func([&](){code;})