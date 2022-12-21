#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

// Secure your code so that each thread can safely enter its full text into the stream.

void do_work(int id, mutex& cout_mutex)
{
    this_thread::sleep_for(10ms);
    string text { "Thread [" };
    text += to_string(id) + "]: " + "Job done!";
    lock_guard guard { cout_mutex };
    cout << text << endl;
}

int main()
{
    mutex cout_mutex;
    vector<thread> threads;
    for (int i = 0; i < 20; i++) {
        threads.emplace_back(thread(do_work, i, std::ref(cout_mutex)));
    }
    for (auto&& t : threads) {
        t.join();
    }
    return 0;
}
