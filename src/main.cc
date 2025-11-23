
#include <coroutine>
#include <print>
#include <list>

class executer {
    public:
        void add_coro(std::coroutine_handle<task::promise_type> c) {
            coros_.push_back(c);
        }

        void run() {
            while (not coros_.empty()) {
                auto current_coro = coros_.front();
                coros_.pop_front();
                current_coro.resume();
            }
        }

    private:
        std::list<std::coroutine_handle<task::promise_type>> coros_;
};

class worker {
    public:
        worker(executer& e)
            : executer_(e) {}
    public:
        bool await_ready() {return false;}
        void await_suspend(std::coroutine_handle<task::promise_type> h) {
            executer_.add_coro(h);
        }
        void await_resume() {}

    private:
        executer& executer_;
};

struct std::coroutine_traits<task, executer*>
    struct promise_type {
        promise_type(executer* e ) 
            : executer_(e) {}

        task get_return_object() {
            task ret(std::coroutine_handle<promise_type>::from_promise(*this));
            ret.set_executer(exeuter_);
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}

        executer* executer_;
    }
};

template<typename... args>
struct std::coroutine_traits<task, args...>
    struct promise_type {
        task get_return_object() {
            return {} //task(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
    }
};

class task {
    public:
        /*
        task(std::coroutine_handle<promise_type> handle) 
            : this_handle_(handle) {}
        */
        task(executer* e)
            : executer_(e) {}

    public:
        bool await_ready() {return the_work_.await_ready();}
        void await_suspend(std::coroutine_handle<task::promise_type> h) {
            the_work_.await_suspend(h);
        }
        void await_resume() { the_work_.await_resume(); }

    private:
        executer* executer_ = nullptr;

    /*
    public:
        void resume() {
            if (not this_handle_.done()) {
                this_handle_.resume();
            }
        }

    private:
        task(std::coroutine_handle<promise_type> handle) 
            : this_handle_(handle) {}

   private:
       std::coroutine_handle<promise_type> this_handle_;
   */
};


task bar() {
    work w;
    co_wait w.do_work();
}

task foo(executer& e) {
    std::println("co awaiting bar");
    co_await bar();
    std::println("work resumed");
}

int main() {
    std::println("main started");

    executer the_runner;
    foo(the_runner);
    std::println("starting main loop");
    the_runner.run();

    std::println("main ended");
}

