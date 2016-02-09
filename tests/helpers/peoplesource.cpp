#include "peoplesource.hpp"
#include <fstream>
#include <iostream>
#include <rex/exceptions.hpp>

PeopleSource::PeopleSource(std::string path):
    mPath(std::move(path))
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
    return {mPersonIndex.begin(), mPersonIndex.end()};
}
