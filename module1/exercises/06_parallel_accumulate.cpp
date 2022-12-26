#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

template <typename It, typename T>
void calcPartForThread(It beg, It end, T& result, std::mutex& mtx)
{
    T partialResult = std::accumulate(beg, end, 0);
    std::lock_guard<std::mutex> lock(mtx);
    result += partialResult;
}
template <typename It>
using ThreadsVectorWithNextPos = std::pair<std::vector<std::thread>, It>;

template <typename It, typename T>
ThreadsVectorWithNextPos<It> runPartCalcsInThreads(It beg, It end, T& result, std::mutex& mtx)
{
    std::vector<std::thread> threads;
    // we want to count last chunk in main thread so -1
    const long int numberOfHwThreads = std::thread::hardware_concurrency() - 1;
    constexpr unsigned minElementsForThread = 200'000;
    const auto numberOfAllElements = std::distance(beg, end);
    const long int neededThreads = std::min(numberOfAllElements / minElementsForThread, numberOfHwThreads);
    const auto chunkSize = numberOfAllElements / numberOfHwThreads;
    for (unsigned i = 0; i < neededThreads; ++i) {
        It nextPartStart = std::next(beg, chunkSize);
        threads.emplace_back(calcPartForThread<It, T>,
                             beg,
                             nextPartStart,
                             std::ref(result),
                             std::ref(mtx));
        beg = nextPartStart;
    }

    return { std::move(threads), beg };
}

template <typename It, typename T>
T parallelAccumulate(It first, It last, T init)
{
    std::mutex resultMtx;
    auto&& [threads, lastChunkStart] = runPartCalcsInThreads(first, last, init, resultMtx);

    // we calc last chunk till the end in main thread
    calcPartForThread(lastChunkStart, last, init, resultMtx);
    for (auto&& th : threads) {
        th.join();
    }

    return init;
}

int main()
{
    // std::vector<int> vec(1'000);   // thousand elements
    // std::vector<int> vec(10'000);   // ten thousand elements
    // std::vector<int> vec(100'000);   // one hundred thousand elements
    std::vector<int> vec(500'000);   // 500 thousand elements
    // std::vector<int> vec(1'000'000);   // million elements
    // std::vector<int> vec(2'000'000);   // 2 mililon elements
    // std::vector<int> vec(10'000'000);   // 10 million elements
    // std::vector<int> vec(100'000'000);   // 100 million elements
    // std::vector<int> vec(1'000'000'000);   // 100 million elements
    std::generate(begin(vec), end(vec), [x { 0 }]() mutable { return ++x; });
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
