#pragma once

namespace rex
{
    template <typename ResourceType>
    struct ResourceView
    {
        const std::string& id;
        const ResourceType& resource;
    };
}
