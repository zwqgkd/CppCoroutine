#include<coroutine>
#include<iostream>


struct Generator {
    class ExhaustedException : std::exception {};

    struct promise_type {
        int value;
        bool is_ready = false;

        std::suspend_always initial_suspend() {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {}

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        //co_yield := co_await promise.yield_value(value)
        std::suspend_always yield_value(int value) {
            this->value = value;
            is_ready = true;
            return {};
        }

        //co_await
        void return_void() {}
    };

    std::coroutine_handle<promise_type> handle;

    explicit Generator(std::coroutine_handle<promise_type> handle) noexcept
        : handle(handle) {}
};

Generator sequence() {
    int i = 0;
    while (i < 5) {
        co_yield i++;
    }
}

Generator returns_generator() {
    auto g = sequence();
    if (g.)
}

int main() {
    auto generator
}