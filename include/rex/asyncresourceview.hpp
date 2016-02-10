#pragma once
#include <string>
#include <future>

namespace rex
{
    template <typename ResourceType>
    struct AsyncResourceView
    {
        std::string identifier;
        std::shared_future<const ResourceType&> future;
    };
}
