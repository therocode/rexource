#pragma once
#include <string>

class PeopleSource
{
    public:
        PeopleSource(std::string path);
        const std::string& path() const;
    private:
        std::string mPath;
};
