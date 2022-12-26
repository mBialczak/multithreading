// TODO: VERIFY includes
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

template <typename It, typename UnaryPredicate>
typename std::iterator_traits<It>::difference_type
    parallel_count_if(It beg, It end, UnaryPredicate pred)
{
    using DiffType = typename std::iterator_traits<It>::difference_type;
    std::vector<std::thread> threads;
    constexpr unsigned minElementsForThread = 100'000;
    const long int numberOfHwThreads = std::thread::hardware_concurrency();
    const auto numberOfAllElements = std::distance(beg, end);
    const long int neededThreads = std::min(numberOfAllElements / minElementsForThread,
                                            numberOfHwThreads);
    if (neededThreads < 2) {
        return std::count_if(beg, end, pred);
    }

    const auto chunkSize = numberOfAllElements / neededThreads;
    std::vector<DiffType> partialResults(neededThreads);
    for (int i = 0; i < neededThreads - 1; ++i) {
        It nextPartStart = std::next(beg, chunkSize);
        threads.emplace_back([beg, nextPartStart, &res = partialResults[i], pred]() mutable {
            res = std::count_if(beg, nextPartStart, pred);
        });
        beg = nextPartStart;
    }
    partialResults[neededThreads - 1] = std::count_if(beg, end, pred);

    for (auto&& th : threads) {
        th.join();
    }

    return std::accumulate(partialResults.begin(), partialResults.end(), DiffType {});
}

int main()
{
    const std::string stringForDivisibleBy3 { "generated for n divisible by three" };
    const std::string stringForDivisibleBy5 { "generated for n divisible by five" };

    // std::vector<std::string> sample(1'000);
    // std::vector<std::string> sample(10'000);
    // std::vector<std::string> sample(100'000);
    // std::vector<std::string> sample(200'000);
    // std::vector<std::string> sample(500'000);
    // std::vector<std::string> sample(750'000);
    // std::vector<std::string> sample(1'000'000);
    // std::vector<std::string> sample(1'600'000);
    // std::vector<std::string> sample(2'000'000);
    // std::vector<std::string> sample(5'000'000);
    // std::vector<std::string> sample(10'000'000);
    std::vector<std::string> sample(20'000'000);

    std::generate(sample.begin(),
                  sample.end(),
                  [=, n { 0 }]() mutable -> std::string {
                      std::string result;
                      if (n % 3 == 0) {
                          result = stringForDivisibleBy3;
                      }
                      else if (n % 5 == 0) {
                          result = stringForDivisibleBy5;
                      }
                      else {
                          result = "standard boring string";
                      }
                      ++n;

                      return result;
                  });

    auto start = std::chrono::steady_clock::now();
    auto parallelResult = parallel_count_if(sample.begin(),
                                            sample.end(),
                                            [=](const auto& elem) {
                                                if (elem == stringForDivisibleBy3
                                                    || elem == stringForDivisibleBy5) {
                                                    return true;
                                                }

                                                return false;
                                            });
    auto stop = std::chrono::steady_clock::now();

    auto start2 = std::chrono::steady_clock::now();
    auto nonParallelResult = std::count_if(sample.begin(),
                                           sample.end(),
                                           [=](const auto& elem) {
                                               if (elem == stringForDivisibleBy3
                                                   || elem == stringForDivisibleBy5) {
                                                   return true;
                                               }

                                               return false;
                                           });
    auto stop2 = std::chrono::steady_clock::now();

    std::cout << "Parallel and non-parallel results are equal: "
              << std::boolalpha << (nonParallelResult == parallelResult)
              << std::endl;
    std::cout << "\nmatches found with normal algorithm: " << nonParallelResult << std::endl;
    std::cout << "\nmatches found with parallel algorithm: " << parallelResult << std::endl;

    const auto parRunTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    const auto seqRunTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2).count();

    std::cout << "\nParallel algorithm: " << parRunTime << " ms" << std::endl;
    std::cout << "\nNormal algorithm: " << seqRunTime << " ms" << std::endl;

    const long double seqToParRatio = static_cast<long double>(seqRunTime) / parRunTime;

    std::cout << "\nNormal algorithm to parallel efficiency ratio: "
              << seqToParRatio
              << std::endl;
}