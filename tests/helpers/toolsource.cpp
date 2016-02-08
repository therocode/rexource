#include "toolsource.hpp"

ToolSource::ToolSource(std::string path):
    mPath(std::move(path))
{
}

const std::string& ToolSource::path() const
{
    return mPath;
}
