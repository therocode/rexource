#include "toolsource.hpp"

ToolSource::ToolSource(std::string path):
    mPath(std::move(path))
{
}

const std::string& ToolSource::path() const
{
    return mPath;
}

Tool ToolSource::load(const std::string& id) const
{
    return Tool();
}

std::vector<std::string> ToolSource::list() const
{
    return {};
}
