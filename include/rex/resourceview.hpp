#pragma once

namespace rex
{
    template <typename ResourceType>
    struct ResourceView
    {
        const std::string& identifier;
        const ResourceType& resource;
    };
}
