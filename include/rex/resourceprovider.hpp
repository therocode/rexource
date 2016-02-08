#pragma once
#include <unordered_map>
#include <thero/any.hpp>
#include <rex/exceptions.hpp>
#include <rex/sourceview.hpp>

namespace rex
{
    class ResourceProvider
    {
        public:
            template <typename SourceType>
            SourceView<SourceType> source(const std::string& sourceId) const;
            template <typename SourceType>
            SourceView<SourceType> addSource(const std::string& sourceId, SourceType source);
            bool removeSource(const std::string& sourceId);
        private:
            template <typename SourceType>
            SourceView<SourceType> sourceIteratorToView(std::unordered_map<std::string, th::Any>::const_iterator iterator) const;
            std::unordered_map<std::string, th::Any> mSources;
    };

    template <typename SourceType>
    SourceView<SourceType> ResourceProvider::source(const std::string& sourceId) const
    {
        auto source = mSources.find(sourceId);

        if(source != mSources.end())
        {
            try
            {
                return sourceIteratorToView<SourceType>(source);
            }
            catch(th::AnyTypeException)
            {
                throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");
            }
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    template <typename SourceType>
    SourceView<SourceType> ResourceProvider::addSource(const std::string& sourceId, SourceType source)
    {
        auto added = mSources.emplace(sourceId, std::move(source));

        if(added.second)
            return sourceIteratorToView<SourceType>(added.first);
        else
            throw InvalidSourceException("adding source id " + sourceId + " which is already added");
    }

    inline bool ResourceProvider::removeSource(const std::string& sourceId)
    {
        return mSources.erase(sourceId) != 0;
    }

    template <typename SourceType>
    SourceView<SourceType> ResourceProvider::sourceIteratorToView(std::unordered_map<std::string, th::Any>::const_iterator iterator) const
    {
        return SourceView<SourceType>
        {
            iterator->first,
            iterator->second.get<SourceType>(),
        };
    }
}
