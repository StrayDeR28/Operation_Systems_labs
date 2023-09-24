#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <list>
#include <condition_variable>

using namespace std;

mutex globalMutex;
condition_variable condVar;

bool ready = true;
bool predF() { return ready == false; };
bool predT() { return ready == true; };
int hub;
list<int> endList;
list<int> startList = { 1,2,3,4,5 };

void Provide(list<int> x) 
{
    unique_lock<mutex> uniqueLock1(globalMutex, defer_lock);
    uniqueLock1.lock();
    condVar.wait(uniqueLock1, predT);
    this_thread::sleep_for(chrono::seconds(1));
    int temp = *x.begin();
    x.pop_front();
    uniqueLock1.unlock();
    hub = temp;
    cout << "Sended element: " << hub << "   Thread id: " << this_thread::get_id() << "\n";
    ready = false;
    condVar.notify_all();
    if (!x.empty()) 
    {
        Provide(x);
    }
}
void Consume() 
{
    unique_lock<mutex> uniqueLock2(globalMutex, defer_lock);
    uniqueLock2.lock();
    condVar.wait(uniqueLock2, predF);
    endList.push_front(hub);
    cout << "Recieved element: " << *endList.begin() << "   Thread id: " << this_thread::get_id() << "\n";
    uniqueLock2.unlock();
    if (endList.size() == startList.size()) { return; };
    ready = true;
    condVar.notify_all();
    Consume();
}
int main()
{
    thread t1(Provide,startList);
    thread t2(Consume);
    t1.join();
    t2.join();
}