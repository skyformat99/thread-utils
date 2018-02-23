#include <iostream>
#include <thread>
#include <string>
#include <future>
#include <chrono>

/*
 * https://stackoverflow.com/questions/21500381/storing-stdpromise-objects-in-a-stdpair
 */

using namespace std::chrono_literals;

typedef void rv_t;

void trad(std::promise<rv_t> && p)
{
    //sleep(1);
    std::this_thread::sleep_for(2s);
    //p.set_value(10);
    p.set_value();
}



int main()
{

    std::promise<rv_t> p;
    std::future<rv_t> f = p.get_future();
    std::thread t(trad, std::move(p));
    //std::thread t(trad, p);

    //std::thread( [&p]{ p.set_value_at_thread_exit(9); }).detach();

    //int x = f.get();
    f.get();

    //std::cout << x << std::endl;
    t.join();

    return 0;
}
