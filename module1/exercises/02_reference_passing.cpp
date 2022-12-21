#include <iostream>
#include <thread>
using namespace std;

void add10(int& a)
{
    a += 10;
    cout << "inside thread: " << a << std::endl;
}

int main()
{
    // run add10 function in a thread
    // pass 5 as an argument and read it's value
    int result { 5 };
    std::thread t1 { add10, std::ref(result) };
    std::cout << "Before join: " << result << std::endl;
    t1.join();
    std::cout << "After join: " << result << std::endl;
    return 0;
}
