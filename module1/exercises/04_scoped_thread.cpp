#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace std;

class scoped_thread
{
  public:
    scoped_thread() noexcept = default;
    // NOTE: for clarity, but would be deleted by default
    scoped_thread(const scoped_thread&) = delete;
    scoped_thread& operator=(const scoped_thread&) = delete;

    // NOTE:  this actually is not needed as it works without it
    explicit scoped_thread(std::thread&& thread)
    {
        if (thread.joinable()) {
            thread_ = std::move(thread);
        }
    }

    scoped_thread(scoped_thread&& other) noexcept = default;

    template <typename Function, typename... Args>
    explicit scoped_thread(Function&& func, Args&&... args)
        : thread_(forward<Function>(func), forward<Args>(args)...)
    { }

    ~scoped_thread()
    {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    scoped_thread& operator=(scoped_thread&& other) noexcept = default;

    bool joinable() const noexcept
    {
        return thread_.joinable();
    }

    std::thread::id get_id() const noexcept
    {
        return thread_.get_id();
    }

    auto native_handle()
    {
        return thread_.native_handle();
    }

    void join()
    {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void detach()
    {
        thread_.detach();
    }

    void swap(scoped_thread& other) noexcept
    {
        thread_.swap(other.thread_);
    }

  private:
    thread thread_;
};

void do_sth(int)
{
    this_thread::sleep_for(1s);
    cout << this_thread::get_id() << '\n';
}

void do_sth_unsafe_in_current_thread()
{
    throw runtime_error("Whoa!");
}

int main()
try {
    scoped_thread st(std::thread(do_sth, 42));
    // auto st2 = st;   // copying not allowed
    [[maybe_unused]] auto st3 = move(st);
    scoped_thread st4(do_sth, 42);

    // NOTE:  added by me to check construction from std::thread
    std::thread added_thread(do_sth, 777);
    scoped_thread scoped_added(std::move(added_thread));
    // end of part added by me

    do_sth_unsafe_in_current_thread();
    return 0;
}
catch (const exception& e) {
    cout << e.what() << endl;
    return -1;
}   // thread is safely destroyed
