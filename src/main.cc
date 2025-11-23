
#include <coroutine>
#include <print>
#include <list>
#include <optional>

class executer {
    public:
        void add_coro(std::coroutine_handle<> c) {
            coros_.push_back(c);
        }

        void run() {
            started_ = true;
            while (not coros_.empty()) {
                auto current_coro = coros_.front();
                coros_.pop_front();
                current_coro.resume();
            }
        }

    public:
        bool await_ready() { return started_; }
        void await_suspend(std::coroutine_handle<> h) {
            add_coro(h);
        }
        bool await_resume() {return started_;}

    public:
        bool started() const {
            return started_;
        }

    private:
        std::list<std::coroutine_handle<>> coros_;
        bool started_ = false;
} main_executer;

template<class T>
class task {
    public:
    using value_type = std::optional<T>;

    class promise_type {
        public:

            task get_return_object() {
                return task(std::coroutine_handle<task::promise_type>::from_promise(*this));
            }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void unhandled_exception() {}
            void return_value(T t) {
                data_ = t;
            }

            value_type& get() { return data_; }

        private:
            value_type data_;
    };

    public:
        task(std::coroutine_handle<task::promise_type> handle) 
            : this_handle_(handle) {}

        bool await_ready() {return get().has_value();}
        void await_suspend(std::coroutine_handle<task::promise_type> h) {
            main_executer.add_coro(h);
        }
        value_type await_resume() {
            return get(); 
        }

    public:
        value_type& get() {
            return this_handle_.promise().get();
        }
    private:
        std::coroutine_handle<promise_type> this_handle_;
};

template<>
class task<void> {
    public:
    class promise_type {
        public:
            task get_return_object() {
                return task(std::coroutine_handle<task::promise_type>::from_promise(*this));
            }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void unhandled_exception() {}
            void return_void() {}
            void get() {}
    };

    public:
        task(std::coroutine_handle<task::promise_type> handle) 
            : this_handle_(handle) {}

        bool await_ready() {return false;}
        void await_suspend(std::coroutine_handle<task::promise_type> h) {
            main_executer.add_coro(h);
        }
        void await_resume() {}

    public:
        void get() {}

    private:
        std::coroutine_handle<promise_type> this_handle_;
};

task<int> executer_started() {
    co_return main_executer.started();
}

task<int> magic_number() {
    co_await executer_started();
    co_return 12;
}

task<int> foo() {
    std::println("co awaiting magic_number()");
    auto number = co_await magic_number();
    std::println("work resumed: {}", number.value_or(-1));
    co_return number.value_or(-1);
}

int main() {
    std::println("main started");

    foo();
    std::println("starting main loop");
    main_executer.run();

    std::println("main ended");
}

