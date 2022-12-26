#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

template <typename It, typename T>
void calcPartForThread(It beg, It end, T& partialResult)
{
    partialResult = std::accumulate(beg, end, T {});
}

template <typename It, typename T>
T parallelAccumulate(It first, It last, T init)
{
    std::vector<std::thread> threads;
    constexpr unsigned minElementsForThread = 200'000;
    const long int numberOfHwThreads = std::thread::hardware_concurrency();
    const auto numberOfAllElements = std::distance(first, last);
    const long int neededThreads = std::min(numberOfAllElements / minElementsForThread, numberOfHwThreads);
    if (neededThreads < 2) {
        return std::accumulate(first, last, init);
    }

    const auto chunkSize = numberOfAllElements / neededThreads;
    std::vector<T> partialResults(neededThreads);

    for (int i = 0; i < neededThreads - 1; ++i) {
        It nextPartStart = std::next(first, chunkSize);
        threads.emplace_back(calcPartForThread<It, T>,
                             first,
                             nextPartStart,
                             std::ref(partialResults[i]));
        first = nextPartStart;
    }

    // we calc last chunk till the end in main thread
    calcPartForThread(first, last, partialResults.back());

    for (auto&& th : threads) {
        th.join();
    }

    return std::accumulate(partialResults.begin(), partialResults.end(), init);
}

int main()
{
    // std::vector<int> vec(1'000);   // thousand elements
    // std::vector<int> vec(10'000);   // ten thousand elements
    // std::vector<int> vec(100'000);   // one hundred thousand elements
    // std::vector<int> vec(300'000);   // 500 thousand elements
    // std::vector<int> vec(500'000);   // 500 thousand elements
    // std::vector<int> vec(1'000'000);   // million elements
    // std::vector<int> vec(2'000'000);   // 2 million elements
    // std::vector<int> vec(10'000'000);   // 10 million elements
    std::vector<int> vec(1'000'000'000);   // 10 million elements

    auto start = std::chrono::steady_clock::now();
    int parallelResult = parallelAccumulate(std::begin(vec), std::end(vec), 0);
    auto stop = std::chrono::steady_clock::now();

    auto start2 = std::chrono::steady_clock::now();
    int nonParallelResult = std::accumulate(std::begin(vec), std::end(vec), 0);
    auto stop2 = std::chrono::steady_clock::now();

    std::cout << "Parallel and non-parallel results are equal: "
              << std::boolalpha << (nonParallelResult == parallelResult)
              << std::endl;

    const auto parRunTime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    const auto seqRunTime = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2).count();

    std::cout << "\nParallel algorithm: " << parRunTime << "us" << std::endl;
    std::cout << "\nNormal algorithm: " << seqRunTime << "us" << std::endl;

    const long double seqToParRatio = static_cast<long double>(seqRunTime) / parRunTime;

    std::cout << "\nNormal algorithm to parallel efficiency ratio: "
              << seqToParRatio
              << std::endl;

    return 0;
}
