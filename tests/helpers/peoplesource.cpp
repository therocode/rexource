#include "peoplesource.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <rex/exceptions.hpp>

PeopleSource::PeopleSource(std::string path, bool simulateLatency):
    mPath(std::move(path)),
    mSimulateLatency(simulateLatency)
{
    std::ifstream inFile(mPath);

    std::string line;

    while(std::getline(inFile, line))
    {
        if(line[0] == '%')
        {
            line.erase(0, 1);

            mPersonIndex.emplace(std::move(line));
        }
    }
}

const std::string& PeopleSource::path() const
{
    return mPath;
}

Person PeopleSource::load(const std::string& id) const
{
    if(mSimulateLatency)
        delay();

    std::ifstream inFile(mPath);

    std::string line;

    while(std::getline(inFile, line))
    {
        if(line[0] == '%')
        {
            line.erase(0, 1);

            if(line == id)
            {
                std::string name = line;

                std::getline(inFile, line);

                int32_t age = std::stoi(line);

                return {name, age};
            }
        }
    }

    throw rex::InvalidResourceException("No such person as '" + id + "'");
}

std::vector<std::string> PeopleSource::list() const
{
    if(mSimulateLatency)
        delay();

    return {mPersonIndex.begin(), mPersonIndex.end()};
}

void PeopleSource::delay() const
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
