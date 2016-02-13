#pragma once
#include <rex/config.hpp>

namespace rex
{
    template <typename ResourceType>
    struct ResourceView
    {
        std::string identifier;
        const ResourceType& resource;
    };
}
