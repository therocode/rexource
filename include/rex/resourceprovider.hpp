#pragma once
#include <rex/config.hpp>
#include <mutex>
#include <unordered_map>

#include <rex/thero.hpp>

#include <rex/asyncresourceview.hpp>
#include <rex/exceptions.hpp>
#include <rex/resourceview.hpp>
#include <rex/sourceview.hpp>
#include <rex/threadpool.hpp>

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
            std::type_index typeProvided;
        };

        public:
            ResourceProvider(int32_t workerCount = 10);
            //sources
            template <typename SourceType>
            SourceView<SourceType> source(const std::string& sourceId) const;
            template <typename SourceType>
            SourceView<SourceType> addSource(const std::string& sourceId, SourceType source);
            std::vector<std::string> sources() const;
            bool removeSource(const std::string& sourceId);
            void clearSources();
            //list
            std::vector<std::string> list(const std::string& sourceId) const;
            //sync get
            template <typename ResourceType>
            const ResourceType& get(const std::string& sourceId, const std::string& resourceId) const;
            template <typename ResourceType>
            std::vector<ResourceView<ResourceType>> get(const std::string& sourceId, const std::vector<std::string>& resourceIds) const;
            template <typename ResourceType>
            std::vector<ResourceView<ResourceType>> getAll(const std::string& sourceId) const;
            //async get
            template <typename ResourceType>
            AsyncResourceView<ResourceType> asyncGet(const std::string& sourceId, const std::string& resourceId) const;
            template <typename ResourceType>
            std::vector<AsyncResourceView<ResourceType>> asyncGet(const std::string& sourceId, const std::vector<std::string>& resourceIds) const;
            template <typename ResourceType>
            std::vector<AsyncResourceView<ResourceType>> asyncGetAll(const std::string& sourceId) const;
            //free
            void markUnused(const std::string& sourceId, const std::string& resourceId);
            void markAllUnused(const std::string& sourceId);
        private:
            template <typename SourceType>
            SourceView<SourceType> sourceIteratorToView(std::unordered_map<std::string, SourceEntry>::const_iterator iterator) const;
            bool resourceReady(const std::string& sourceId, const std::string& resourceId) const;
            bool resourceLoadInProgress(const std::string& sourceId, const std::string& resourceId) const;
            template <typename ResourceType>
            const ResourceType& loadGetResource(const std::string& sourceId, const std::string& resourceId) const;
            std::unordered_map<std::string, SourceEntry> mSources;
            mutable std::unordered_map<std::string, std::unordered_map<std::string, th::Any>> mResources;
            mutable std::unordered_map<std::string, std::unordered_map<std::string, th::Any>> mAsyncProcesses;
            mutable std::mutex mResourceMutex;
            mutable ThreadPool mThreadPool;
    };

    inline ResourceProvider::ResourceProvider(int32_t workerCount):
        mThreadPool(workerCount)
    {
    }

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
        using ResourceType = decltype(source.load(std::string()));
        LoadingFunction<ResourceType> loadingFunction = [] (const th::Any& packedSource, const std::string& identifier)
        {
            return packedSource.get<SourceType>().load(identifier);
        };

        ListingFunction listingFunction = [] (const th::Any& packedSource)
        {
            return packedSource.get<SourceType>().list();
        };

        auto added = mSources.emplace(sourceId, SourceEntry{std::move(source), loadingFunction, listingFunction, typeid(ResourceType)});
        mResources[sourceId];
        mAsyncProcesses[sourceId];

        if(added.second)
            return sourceIteratorToView<SourceType>(added.first);
        else
            throw InvalidSourceException("adding source id " + sourceId + " which is already added");
    }

    inline std::vector<std::string> ResourceProvider::sources() const
    {
        std::vector<std::string> result;

        for(const auto& source : mSources)
           result.push_back(source.first); 

        return result;
    }

    inline bool ResourceProvider::removeSource(const std::string& sourceId)
    {
        mResources.erase(sourceId);
        mAsyncProcesses.erase(sourceId);

        return mSources.erase(sourceId) != 0;
    }

    inline void ResourceProvider::clearSources()
    {
        mSources.clear();
        mResources.clear();
        mAsyncProcesses.clear();
    }

    inline std::vector<std::string> ResourceProvider::list(const std::string& sourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            const auto& source = sourceIterator->second.source;

            return sourceIterator->second.listingFunction(source);
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    template <typename ResourceType>
    const ResourceType& ResourceProvider::get(const std::string& sourceId, const std::string& resourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            const auto& source = sourceIterator->second.source;

            if(std::type_index(typeid(ResourceType)) != sourceIterator->second.typeProvided)
                throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");

            return ResourceProvider::loadGetResource<ResourceType>(sourceId, resourceId);
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    template <typename ResourceType>
    std::vector<ResourceView<ResourceType>> ResourceProvider::get(const std::string& sourceId, const std::vector<std::string>& resourceIds) const
    {
        std::vector<ResourceView<ResourceType>> result;

        for(const std::string& resourceId : resourceIds)
            result.emplace_back(ResourceView<ResourceType>{resourceId, get<ResourceType>(sourceId, resourceId)});

        return result;
    }

    template <typename ResourceType>
    std::vector<ResourceView<ResourceType>> ResourceProvider::getAll(const std::string& sourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            const auto& source = sourceIterator->second.source;

            std::vector<std::string> idList = sourceIterator->second.listingFunction(source);

            return get<ResourceType>(sourceId, idList);
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    template <typename ResourceType>
    AsyncResourceView<ResourceType> ResourceProvider::asyncGet(const std::string& sourceId, const std::string& resourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            if(std::type_index(typeid(ResourceType)) != sourceIterator->second.typeProvided)
                throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");

            bool resourceIsReady = false;

            {
                std::lock_guard<std::mutex> lock(mResourceMutex);
                resourceIsReady = resourceReady(sourceId, resourceId);
            }

            if(resourceIsReady)
            {//the resource is ready and this won't change from another thread
                //make and return instantly valid future

                throw std::runtime_error("oops oops");
            }
            else
            {
                {
                    std::lock_guard<std::mutex> lock(mResourceMutex);

                    if(resourceLoadInProgress(sourceId, resourceId))
                    {//there is a future ready to piggyback on
                        return AsyncResourceView<ResourceType>{resourceId, mAsyncProcesses.at(sourceId).at(resourceId).get<std::shared_future<const ResourceType&>>()};
                    }
                }

                //if we reached here, it means that there is no currently loaded resource and no process to load it, and this won't change, so it is safe to start loading
                auto boundLaunch = std::bind(&ResourceProvider::loadGetResource<ResourceType>, this, sourceId, resourceId);
                std::shared_future<const ResourceType&> futureResource = mThreadPool.enqueue(std::move(boundLaunch), 0);

                {//other threads can still affect the storage on other resources though so we better lock
                    std::lock_guard<std::mutex> lock(mResourceMutex);

                    auto emplaced = mAsyncProcesses.at(sourceId).emplace(resourceId, std::move(futureResource));
                    return AsyncResourceView<ResourceType>{resourceId, emplaced.first->second.template get<std::shared_future<const ResourceType&>>()};
                }
            }
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    template <typename ResourceType>
    std::vector<AsyncResourceView<ResourceType>> ResourceProvider::asyncGet(const std::string& sourceId, const std::vector<std::string>& resourceIds) const
    {
        std::vector<AsyncResourceView<ResourceType>> result;

        for(const std::string& resourceId : resourceIds)
            result.emplace_back(asyncGet<ResourceType>(sourceId, resourceId));

        return result;
    }

    template <typename ResourceType>
    std::vector<AsyncResourceView<ResourceType>> ResourceProvider::asyncGetAll(const std::string& sourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            const auto& source = sourceIterator->second.source;

            std::vector<std::string> idList = sourceIterator->second.listingFunction(source);

            return asyncGet<ResourceType>(sourceId, idList);
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    inline void ResourceProvider::markUnused(const std::string& sourceId, const std::string& resourceId)
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            //TODO wait for futures before deleting
            std::lock_guard<std::mutex> lock(mResourceMutex);
            mResources.at(sourceId).erase(resourceId);
            mAsyncProcesses.at(sourceId).erase(resourceId);
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }

    inline void ResourceProvider::markAllUnused(const std::string& sourceId)
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            //TODO wait for futures before deleting
            std::lock_guard<std::mutex> lock(mResourceMutex);
            mResources.at(sourceId).clear();
            mAsyncProcesses.at(sourceId).clear();
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
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

    inline bool ResourceProvider::resourceReady(const std::string& sourceId, const std::string& resourceId) const
    {
        auto sourceIterator = mResources.find(sourceId);

        if(sourceIterator != mResources.end())
            return sourceIterator->second.count(resourceId) != 0;

        return false;
    }

    inline bool ResourceProvider::resourceLoadInProgress(const std::string& sourceId, const std::string& resourceId) const
    {
        auto sourceIterator = mAsyncProcesses.find(sourceId);

        if(sourceIterator != mAsyncProcesses.end())
            return sourceIterator->second.count(resourceId);

        return false;
    }

    //NOTE: should be only a loader. get<> has to check existence before loading, and wait for processes? etc etc etc etc
    template <typename ResourceType>
    const ResourceType& ResourceProvider::loadGetResource(const std::string& sourceId, const std::string& resourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            try
            {
                //lock needed here for resourceReady ??? YES it is, and it won't change?? not needed at all if up there is true ^^^^^^^
                if(!resourceReady(sourceId, resourceId))
                {
                    auto loadFunction = sourceIterator->second.loadingFunction.get<LoadingFunction<ResourceType>>();
                    auto resource = loadFunction(sourceIterator->second.source, resourceId);

                    {
                        std::lock_guard<std::mutex> lock(mResourceMutex);
                        auto emplaced = mResources.at(sourceId).emplace(resourceId, std::move(resource));

                        //TODO: remove async process!!!!!!!!!!
                        return emplaced.first->second.template get<ResourceType>();
                    }
                }
                else
                {
                    std::lock_guard<std::mutex> lock(mResourceMutex);
                    return mResources.at(sourceId).at(resourceId).template get<ResourceType>();
                }
            }
            catch(const std::exception& exception)
            {
                throw InvalidResourceException(exception.what());
            }
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }
}
