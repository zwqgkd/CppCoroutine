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

        //co_return调用的函数 return_void, return_value
        void return_void() {}
    };

    std::coroutine_handle<promise_type> handle;

    explicit Generator(std::coroutine_handle<promise_type> handle) noexcept
        : handle(handle) {}

    //不允许拷贝，防止handle空悬
    Generator(const Generator&) = delete;

    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& other)noexcept :handle(std::exchange(other.handle, {})) {}

    ~Generator() {
        if (handle) handle.destroy();
    }

    bool has_next() {
        if (handle.done()) {
            return false;
        }

        if (!handle.promise().is_ready) {
            //让序列继续执行
            handle.resume();
        }
        //序列执行完毕或者再次挂起之后回到这里
        if (handle.done()) {
            return false;
        }
        else {
            return true;
        }
    }

    int next() {
        if (has_next()) {
            handle.promise().is_ready = false;
            return handle.promise().value;
        }
        throw ExhaustedException{};
    }
};

Generator fibonacci() {
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
    auto generator = fibonacci();
    for (int i = 0; i < 10; i++) {
        if (generator.has_next()) {
            std::cout << generator.next() << std::endl;
        }
        else {
            break;
        }
    }
    return 0;
}