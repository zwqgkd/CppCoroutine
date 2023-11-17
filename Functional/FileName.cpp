#include<coroutine>
#include<iostream>
#include<functional>
#include<string>
#include<list>
#include<initializer_list>


template<typename T>
struct Generator {
	class ExhaustedException :std::exception {};

	struct promise_type {
		T value;
		bool is_ready = false;
		std::suspend_always initial_suspend() {
			return {};
		}

		std::suspend_always final_suspend() noexcept{
			return {};
		}

		std::suspend_always yield_value(T value) {
			this->value = value;
			is_ready = true;
			return {};
		}

		void unhandled_exception() {

		}

		Generator get_return_object() {
			return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		void return_void() {}
	};

	std::coroutine_handle<promise_type> handle;

	bool has_next() {
		if (handle.done()) {
			return false;
		}

		if (!handle.promise().is_ready) {
			handle.resume();
		}

		if (handle.done()) {
			return false;
		}
		else {
			return true;
		}
	}

	T next() {
		if (has_next()) {
			handle.promise().is_ready = false;
			return handle.promise().value;
		}
		throw ExhaustedException();
	}

	Generator static from_array(T array[], int n) {
		for (int i = 0; i < n; ++i) {
			co_yield array[i];
		}
	}

	Generator static from_list(std::list<T> list) {
		for (auto t : list) {
			co_yield t;
		}
	}

	Generator static from(std::initializer_list<T> args) {
		for (auto t : args) {
			co_yield t;
		}
	}

	template<typename... Targs>
	Generator static from(Targs... args) {
		(co_yield args, ...);
	}

	explicit Generator(std::coroutine_handle<promise_type> handle) :handle(handle) {}

	Generator(Generator&& other) noexcept :handle(std::exchange(other.handle, {})) {}

	Generator(const Generator&) = delete;

	Generator& operator=(const Generator&) = delete;

	~Generator() {
		if (handle) handle.destroy();
	}

	//map
	template<typename U>
	Generator<U> map1(std::function <U(T)>f) {
		while (has_next()) {
			co_yield f(next());
		}
	}
	
	template<typename F>
	Generator<std::invoke_result_t<F, T>> map2(F f) {
		while (has_next()) {
			co_yield f(next());
		}
	}

	//����һ��Э����
	template<typename F>
	std::invoke_result_t<F, T> flat_map(F f) {
		while (has_next()) {
			//ֵӳ����µ�Э����
			auto generator = f(next());
			//�᲻����ֵ��֮ǰ��������԰�ֵ��yield��ȥ��
			//
			while (generator.has_next()) {
				co_yield generator.next();
			}
		}
	}
};


Generator<int> fibonacci() {
	co_yield 0;
	co_yield 1;

	int a = 0;
	int b = 1;
	while (true) {
		co_yield a + b;
		b = a + b;
		a = b - a;
	}
}
int main() {
	auto func_int2str = [](int i) {
		return std::to_string(i);
	};
	Generator<std::string> generator1_str = fibonacci().map1<std::string>(func_int2str);
	Generator<std::string> generator_str = fibonacci().map2(func_int2str);

	//Э��һ��Ҫ��ʾ��д�����÷�������
	auto func_generator = [](int i)->Generator<int> {
		for (int j = 0; j < i; ++j) {
			co_yield j;
		}
		};

	Generator<int>::from(1, 2, 3, 4).flat_map(func_generator);

	return 0;
}