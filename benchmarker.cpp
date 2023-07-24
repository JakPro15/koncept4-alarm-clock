#include <iostream>
#include <chrono>


int main(int argc, char *argv[])
{
    auto start = std::chrono::steady_clock::now();
    for(int i = 0; i < std::stoul(argv[2]); i++)
    {
        system(argv[1]);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count() << "s\n";
}
