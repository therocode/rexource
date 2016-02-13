#pragma once
#include <rex/config.hpp>

namespace rex
{
    template <typename SourceType>
    struct SourceView
    {
        const std::string& id;
        const SourceType& source;
    };
}
