#pragma once
#include <vector>
#include <string>

namespace rex
{
    template <typename ResourceType>
    class FileSource
    {
        public:
            Person load(const std::string& id) const;
            std::vector<std::string> list() const;
    };
};
