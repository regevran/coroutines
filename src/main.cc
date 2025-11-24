
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

    private:
        task(std::coroutine_handle<task::promise_type> handle) : this_handle_(handle) {}

    public:
        void suspend() { main_executer.add_coro(this_handle_); }
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

    private:
        task(std::coroutine_handle<task::promise_type> handle) : this_handle_(handle) {}

    public:
        void suspend() { main_executer.add_coro(this_handle_); }
        void get() {}
        void set_ready() {
            ready_ = true;
        }
        bool ready() const {
            return ready_;
        }

    private:
        std::coroutine_handle<promise_type> this_handle_;
        bool ready_ = true;
};

template<class T>
struct awaiter {
    using value_type = std::optional<T>;
    awaiter(task<T>& t) : task_(t) {}
    bool await_ready() {return task_.get().has_value();}
    void await_suspend(std::coroutine_handle<>) {
        task_.suspend();
    }
    value_type await_resume() {
        return task_.get(); 
    }

    task<T> task_;
};

template<>
struct awaiter<void> {
    awaiter(task<void>& t) : task_(t) {}
    bool await_ready() {return task_.ready();}
    void await_suspend(std::coroutine_handle<>) {
        task_.suspend();
    }
    void await_resume() {}

    task<void> task_;
};

template <class T>
awaiter<T> operator co_await(task<T> t){
    return awaiter(t);
}

task<bool> executer_started() {
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

