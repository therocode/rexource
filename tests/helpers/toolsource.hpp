#pragma once
#include <string>
#include "tool.hpp"
#include <vector>

class ToolSource
{
    public:
        ToolSource(std::string path);
        const std::string& path() const;
        Tool load(const std::string& id) const;
        std::vector<std::string> list() const;
    private:
        std::string mPath;
};
