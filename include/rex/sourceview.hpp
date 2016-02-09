#pragma once

namespace rex
{
    template <typename SourceType>
    struct SourceView
    {
        const std::string& id;
        const SourceType& source;
    };
}
