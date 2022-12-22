#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::literals::chrono_literals;

void printThreadId(std::mutex& mtx)
{
    std::lock_guard lock(mtx);
    std::cout << std::this_thread::get_id() << "\n";
}

template <typename Duration>
void printThread(int repetitions, Duration duration, std::mutex& mtx)
{
    for (int i = 0; i < repetitions; ++i) {
        printThreadId(mtx);
    }
    std::this_thread::sleep_for(duration);
}

int main()
{
    constexpr int threadsNumber { 4 };
    constexpr int repetitions { 12 };
    constexpr auto duration = 5s;
    std::mutex coutMtx;
    std::vector<std::thread> threads;
    for (int i = 0; i < threadsNumber; ++i) {
        threads.emplace_back(printThread<decltype(duration)>, repetitions, duration, std::ref(coutMtx));
    }

    for (auto&& th : threads) {
        th.join();
    }
}
