#pragma once
#include <unordered_map>
#include <thero/any.hpp>
#include <rex/exceptions.hpp>
#include <rex/sourceview.hpp>
#include <rex/resourceview.hpp>

namespace rex
{
    class ResourceProvider
    {
        template<typename ResourceType>
        using LoadingFunction = ResourceType(*)(const th::Any&, const std::string&);
        using ListingFunction = std::vector<std::string>(*)(const th::Any&);

        struct SourceEntry
        {
            th::Any source;
            th::Any loadingFunction;
            ListingFunction listingFunction;
        };

        public:
            template <typename SourceType>
            SourceView<SourceType> source(const std::string& sourceId) const;
            template <typename SourceType>
            SourceView<SourceType> addSource(const std::string& sourceId, SourceType source);
            bool removeSource(const std::string& sourceId);
            void clearSources();
            template <typename ResourceType>
            const ResourceType& get(const std::string& sourceId, const std::string& resourceId) const;
            template <typename ResourceType>
            std::vector<ResourceView<ResourceType>> getAll(const std::string& sourceId) const;
        private:
            template <typename SourceType>
            SourceView<SourceType> sourceIteratorToView(std::unordered_map<std::string, SourceEntry>::const_iterator iterator) const;
            std::unordered_map<std::string, SourceEntry> mSources;
            mutable std::unordered_map<std::string, std::unordered_map<std::string, th::Any>> mResources;
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
        LoadingFunction<decltype(source.load(std::string()))> loadingFunction = [] (const th::Any& packedSource, const std::string& identifier)
        {
            return packedSource.get<SourceType>().load(identifier);
        };

        ListingFunction listingFunction = [] (const th::Any& packedSource)
        {
            return packedSource.get<SourceType>().list();
        };

        auto added = mSources.emplace(sourceId, SourceEntry{std::move(source), loadingFunction, listingFunction});

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
    SourceView<SourceType> ResourceProvider::sourceIteratorToView(std::unordered_map<std::string, SourceEntry>::const_iterator iterator) const
    {
        return SourceView<SourceType>
        {
            iterator->first,
            iterator->second.source.get<SourceType>(),
        };
    }

    inline void ResourceProvider::clearSources()
    {
        mSources.clear();
    }

    template <typename ResourceType>
    const ResourceType& ResourceProvider::get(const std::string& sourceId, const std::string& resourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            const auto& source = sourceIterator->second.source;

            try
            {
                auto loadFunction = sourceIterator->second.loadingFunction.get<LoadingFunction<ResourceType>>();

                try
                {
                    auto resource = loadFunction(source, resourceId);

                    auto emplaced = mResources[sourceId].emplace(resourceId, std::move(resource));

                    return emplaced.first->second.get<ResourceType>();
                }
                catch(const std::exception& exception)
                {
                    throw InvalidResourceException(exception.what());
                }
            }
            catch(th::AnyTypeException)
            {
                throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");
            }
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    template <typename ResourceType>
    std::vector<ResourceView<ResourceType>> ResourceProvider::getAll(const std::string& sourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            const auto& source = sourceIterator->second.source;

            std::vector<std::string> idList = sourceIterator->second.listingFunction(source);

            try
            {
                std::vector<ResourceView<ResourceType>> result;

                for(size_t i = 0; i < idList.size(); ++i)
                {
                    result.emplace_back(ResourceView<ResourceType>{idList[i], std::move(get<ResourceType>(sourceId, idList[i]))});
                }

                return result;
            }
            catch(th::AnyTypeException)
            {
                throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");
            }
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }
}
