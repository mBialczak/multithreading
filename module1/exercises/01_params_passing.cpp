#include <iostream>
#include <thread>
using namespace std;

int add(int a, int b)
{
    return a + b;
}

int main()
{
    std::thread th_add { add, 3, 4 };
    th_add.join();
    // run add function in a thread
    // pass 3 and 4 as arguments
    return 0;
}
