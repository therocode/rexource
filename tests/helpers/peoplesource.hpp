#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include "person.hpp"

class PeopleSource
{
    public:
        PeopleSource(std::string path);
        const std::string& path() const;
        Person load(const std::string& id) const;
        std::vector<std::string> list() const;
    private:
        std::string mPath;
        std::unordered_set<std::string> mPersonIndex;
};
