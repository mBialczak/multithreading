#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main()
{
    using namespace std::literals::chrono_literals;

    std::vector<std::thread> threads;
    threads.reserve(20);
    constexpr int threadsNumber { 20 };
    std::mutex mtx;

    for (int i = 0; i < threadsNumber; ++i) {
        threads.emplace_back([&mtx](int id) {
            std::this_thread::sleep_for(1s);
            // std::lock_guard<std::mutex> lock(mtx);
            std::cout << id << "\n";
        },
                             i);
    }

    for (int i = 0; i < threadsNumber; ++i) {
        threads[i].join();
    }

    return 0;
}