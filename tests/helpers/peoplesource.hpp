#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include "person.hpp"

class PeopleSource
{
    public:
        PeopleSource(std::string path, bool simulateLatency);
        const std::string& path() const;
        Person load(const std::string& id) const;
        std::vector<std::string> list() const;
    private:
        void delay() const;
        std::string mPath;
        bool mSimulateLatency;
        std::unordered_set<std::string> mPersonIndex;
};
