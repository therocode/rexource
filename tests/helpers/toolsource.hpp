#pragma once
#include <string>

class ToolSource
{
    public:
        ToolSource(std::string path);
        const std::string& path() const;
    private:
        std::string mPath;
};
