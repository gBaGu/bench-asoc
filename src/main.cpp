#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>

#include <boost/container/flat_map.hpp>

using namespace std::chrono;

using boost::container::flat_map;


class Timer
{
public:
    Timer() : start(system_clock::now()) {}

    system_clock::duration timeSinceStart() const
    {
        return system_clock::now() - start;
    }

private:
    system_clock::time_point start;
};


void help()
{
    std::cout << "Usage: <command> [args...]\n"
        << "Available commands:\n"
        << "\tgenerate - generate net data set\n"
        << "\tbenchmark - benchmark all containers"
        << std::endl;
}

void generateData(const std::string& filename, int selectionSize, int stringSize)
{
    std::ofstream fout(filename);
    if (!fout.is_open()) {
        return;
    }

    for (int i = 0; i < selectionSize; i++) {
        std::string str(stringSize, 48 + i % 10);
        fout << i << " " << str << std::endl;
    }
}

std::map<std::uintmax_t, std::string> readData(const std::string& filename)
{
    std::map<std::uintmax_t, std::string> ret;

    std::ifstream fin(filename);
    if (!fin.is_open()) {
        return ret;
    }

    std::uintmax_t id = 0;
    std::string str;
    while (fin >> id >> str) {
        ret[id] = str;
    }

    return ret;
}

template <typename Container>
void benchmark(const std::map<std::uintmax_t, std::string>& data, Container& container)
{
    std::uintmax_t totalInsertTime = 0;
    for (const auto& p: data) {
        Timer timer;
        container.insert(std::make_pair(p.first, p.second));
        const auto insertTime = timer.timeSinceStart();
        totalInsertTime += duration_cast<microseconds>(insertTime).count();
    }

    std::uintmax_t numElements = 0;
    std::uintmax_t currentId = 0;
    Timer timer;
    for (const auto& p: container) {
        numElements++;
        currentId = p.first;
    }
    const auto iterationTime = duration_cast<microseconds>(timer.timeSinceStart()).count();

    std::cout << "Total insert time: " << totalInsertTime << "µs\n"
        << "Avg insert time: " << static_cast<double>(totalInsertTime) / container.size() << "µs\n"
        << "Iteration time: " << iterationTime << "µs" << std::endl;
}


int main(int argc, char ** argv)
{
    if (argc < 2) {
        help();
        return 0;
    }
    std::string command(argv[1]);
    if (command == "help") {
        help();
        return 0;
    }
    else if (command == "generate") {
        if (argc < 5) {
            help();
            return 0;
        }
        std::string filename(argv[2]);
        int selectionSize = std::stoi(argv[3]);
        int stringSize = std::stoi(argv[4]);

        generateData(filename, selectionSize, stringSize);
    }
    else if (command == "benchmark") {
        if (argc < 3) {
            help();
            return 0;
        }
        std::string filename(argv[2]);

        std::map<std::uintmax_t, std::string>            container1;
        std::unordered_map<std::uintmax_t, std::string>  container2;
        flat_map<std::uintmax_t, std::string>            container3;

        const auto data = readData(filename);
        std::cout << "std::map\n";
        benchmark(data, container1);
        container1.clear();
        std::cout << "std::unordered_map\n";
        benchmark(data, container2);
        container2.clear();
        std::cout << "flat_map\n";
        benchmark(data, container3);
        container3.clear();
    }

    return 0;
}