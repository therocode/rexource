#include "peoplesource.hpp"

PeopleSource::PeopleSource(std::string path):
    mPath(std::move(path))
{
}

const std::string& PeopleSource::path() const
{
    return mPath;
}
