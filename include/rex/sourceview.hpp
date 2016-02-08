#pragma once

namespace rex
{
    template <typename SourceType>
    struct SourceView
    {
        const std::string& id;
        const SourceType& source;
    };

    template <typename SourceType>
    bool operator==(const SourceView<SourceType>& a, const SourceView<SourceType>& b)
    {
        return a.id == b.id;
    }

    template <typename SourceType>
    bool operator!=(const SourceView<SourceType>& a, const SourceView<SourceType>& b)
    {
        return a.id != b.id;
    }
}
