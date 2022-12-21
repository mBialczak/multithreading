#include <iostream>
#include <string>
#include <thread>
using namespace std;

class Car
{
    int production_year;
    string model_name;

  public:
    void setData(int year, const string& model)
    {
        production_year = year;
        model_name = model;
    }
    void print()
    {
        cout << model_name << " " << production_year << endl;
    }
};

int main()
{
    Car toyota;
    // set year to 2015, model to "Corolla" in a thread
    thread t1 { &Car::setData, toyota, 2015, "Corolla" };
    t1.join();
    toyota.print();
    return 0;
}
