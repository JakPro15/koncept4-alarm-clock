#include <thread>
#include <string>
#include <cassert>
#include <vector>


unsigned long long stress(unsigned long long number)
{
    unsigned long long sum = 0;
    for(unsigned long long i = 0; i < number; i++)
    {
        sum += i * i;
    }
    return sum;
}


int main(int argc, char *argv[])
{
    int no_threads = std::stoul(argv[1]);
    assert(no_threads > 0);

    std::vector<std::thread> threads;
    for(int i = 0; i < no_threads; i++)
        threads.emplace_back(stress, 1'000'000'000'000);
    for(int i = 0; i < no_threads; i++)
        threads[i].join();
}
