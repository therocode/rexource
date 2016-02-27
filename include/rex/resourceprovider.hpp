#pragma once
#include <rex/config.hpp>

#ifndef REX_DISABLE_ASYNC
#include <mutex>
#endif 

#include <unordered_map>

#include <rex/thero.hpp>

#include <rex/exceptions.hpp>
#include <rex/exceptions.hpp>
#include <rex/resourceview.hpp>
#include <rex/sourceview.hpp>

#ifndef REX_DISABLE_ASYNC
#include <rex/asyncresourceview.hpp>
#include <rex/threadpool.hpp>
#endif 

namespace rex
{
    class ResourceProvider
    {
        template<typename ResourceType>
        using LoadingFunction = ResourceType(*)(const th::Any&, const std::string&);
        using ListingFunction = std::vector<std::string>(*)(const th::Any&);
        using WaitFunction = void(*)(const th::Any&);

        struct SourceEntry
        {
            th::Any source;
            th::Any loadingFunction;
            ListingFunction listingFunction;
            WaitFunction waitFunction;
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
#ifndef REX_DISABLE_ASYNC
            //async get
            template <typename ResourceType>
            AsyncResourceView<ResourceType> asyncGet(const std::string& sourceId, const std::string& resourceId) const;
            template <typename ResourceType>
            std::vector<AsyncResourceView<ResourceType>> asyncGet(const std::string& sourceId, const std::vector<std::string>& resourceIds) const;
            template <typename ResourceType>
            std::vector<AsyncResourceView<ResourceType>> asyncGetAll(const std::string& sourceId) const;
#endif
            //free
            void markUnused(const std::string& sourceId, const std::string& resourceId);
            void markAllUnused(const std::string& sourceId);
        private:
            bool resourceReady(const std::string& sourceId, const std::string& resourceId) const;
            bool resourceLoadInProgress(const std::string& sourceId, const std::string& resourceId) const;
            template <typename ResourceType>
            const ResourceType& loadResource(const std::string& sourceId, const std::string& resourceId) const;
            void waitForSourceAsync(const std::string& sourceId) const;
            const SourceEntry& toSourceEntry(const std::string& sourceId) const;
            std::unordered_map<std::string, SourceEntry> mSources;
            mutable std::unordered_map<std::string, std::unordered_map<std::string, th::Any>> mResources;
#ifndef REX_DISABLE_ASYNC
            mutable std::unordered_map<std::string, std::unordered_map<std::string, th::Any>> mAsyncProcesses;
            mutable std::recursive_mutex mResourceMutex;
            mutable ThreadPool mThreadPool;
#endif
    };

    inline ResourceProvider::ResourceProvider(int32_t workerCount)
#ifndef REX_DISABLE_ASYNC
        :mThreadPool(workerCount)
#endif
    {
    }

    template <typename SourceType>
    SourceView<SourceType> ResourceProvider::source(const std::string& sourceId) const
    {
        const auto& sourceEntry = toSourceEntry(sourceId);

        try
        {
            return SourceView<SourceType>
            {
                sourceId,
                sourceEntry.source.get<SourceType>()
            };
        }
        catch(th::AnyTypeException)
        {
            throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");
        }
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

        WaitFunction waitFunction = [] (const th::Any& packedFuture)
        {
#ifndef REX_DISABLE_ASYNC
            packedFuture.get<std::shared_future<const ResourceType&>>().wait();
#endif
        };

        auto added = mSources.emplace(sourceId, SourceEntry{std::move(source), loadingFunction, listingFunction, waitFunction, typeid(ResourceType)});
        mResources[sourceId];
#ifndef REX_DISABLE_ASYNC
        mAsyncProcesses[sourceId];
#endif

        if(added.second)
            return SourceView<SourceType>
            {
                sourceId,
                added.first->second.source.get<SourceType>()
            };
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
#ifndef REX_DISABLE_ASYNC
        mAsyncProcesses.erase(sourceId);
#endif

        return mSources.erase(sourceId) != 0;
    }

    inline void ResourceProvider::clearSources()
    {
        mSources.clear();
        mResources.clear();
#ifndef REX_DISABLE_ASYNC
        mAsyncProcesses.clear();
#endif
    }

    inline std::vector<std::string> ResourceProvider::list(const std::string& sourceId) const
    {
        const auto& sourceEntry = toSourceEntry(sourceId);

        return sourceEntry.listingFunction(sourceEntry.source);
    }

    template <typename ResourceType>
    const ResourceType& ResourceProvider::get(const std::string& sourceId, const std::string& resourceId) const
    {
        const auto& sourceEntry = toSourceEntry(sourceId);

        const auto& source = sourceEntry.source;

        if(std::type_index(typeid(ResourceType)) != sourceEntry.typeProvided)
            throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");

#ifndef REX_DISABLE_ASYNC
        std::shared_future<const ResourceType&> futureToWaitFor;

        {
            std::lock_guard<std::recursive_mutex> lock(mResourceMutex);

            //there are three possible cases, and with the lock on, these won't change
            
            //1. it is already loaded, so just return it
            if(mResources.at(sourceId).count(resourceId) != 0)
                return mResources.at(sourceId).at(resourceId).get<ResourceType>();

            //2. it is not loaded and no process is loading it
            if(mAsyncProcesses.at(sourceId).count(resourceId) == 0)
                return loadResource<ResourceType>(sourceId, resourceId);

            //3. it is not loaded and there is a process that loads it already
            futureToWaitFor = mAsyncProcesses.at(sourceId).at(resourceId).get<std::shared_future<const ResourceType&>>();
        }

        futureToWaitFor.wait();

        return futureToWaitFor.get();
#else
        //Without async, it is either loaded or not, so just return it or load-return it
        if(mResources.at(sourceId).count(resourceId) != 0)
            return mResources.at(sourceId).at(resourceId).get<ResourceType>();
		else
			return loadResource<ResourceType>(sourceId, resourceId);
#endif
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
        const auto& sourceEntry = toSourceEntry(sourceId);

        std::vector<std::string> idList = sourceEntry.listingFunction(sourceEntry.source);

        return get<ResourceType>(sourceId, idList);
    }

#ifndef REX_DISABLE_ASYNC
    template <typename ResourceType>
    AsyncResourceView<ResourceType> ResourceProvider::asyncGet(const std::string& sourceId, const std::string& resourceId) const
    {
        const auto& sourceEntry = toSourceEntry(sourceId);

        if(std::type_index(typeid(ResourceType)) != sourceEntry.typeProvided)
            throw InvalidSourceException("trying to access source id " + sourceId + " as the wrong type");

        bool resourceIsReady = false;

        {
            std::lock_guard<std::recursive_mutex> lock(mResourceMutex);
            resourceIsReady = resourceReady(sourceId, resourceId);
        }

        if(resourceIsReady)
        {//the resource is ready and this won't change from another thread
            std::promise<const ResourceType&> promise;
            std::shared_future<const ResourceType&> future = promise.get_future();
            promise.set_value(mResources.at(sourceId).at(resourceId).get<ResourceType>());
            return {resourceId, future};
        }
        else
        {
            {
                std::lock_guard<std::recursive_mutex> lock(mResourceMutex);

                if(resourceLoadInProgress(sourceId, resourceId))
                {//there is a future ready to piggyback on
                    return AsyncResourceView<ResourceType>{resourceId, mAsyncProcesses.at(sourceId).at(resourceId).get<std::shared_future<const ResourceType&>>()};
                }
            }

            //if we reached here, it means that there is no currently loaded resource and no process to load it, and this won't change, so it is safe to start loading
            {//other threads can still affect the storage on other resources though so we better lock
                std::lock_guard<std::recursive_mutex> lock(mResourceMutex);

                auto boundLaunch = std::bind(&ResourceProvider::loadResource<ResourceType>, this, sourceId, resourceId);
                std::shared_future<const ResourceType&> futureResource = mThreadPool.enqueue(std::move(boundLaunch), 0);

                auto emplaced = mAsyncProcesses.at(sourceId).emplace(resourceId, std::move(futureResource));
                return AsyncResourceView<ResourceType>{resourceId, emplaced.first->second.template get<std::shared_future<const ResourceType&>>()};
            }
        }
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
        const auto& sourceEntry = toSourceEntry(sourceId);

        std::vector<std::string> idList = sourceEntry.listingFunction(sourceEntry.source);

        return asyncGet<ResourceType>(sourceId, idList);
    }
#endif

    inline void ResourceProvider::markUnused(const std::string& sourceId, const std::string& resourceId)
    {
        const auto& sourceEntry = toSourceEntry(sourceId);
#ifndef REX_DISABLE_ASYNC
        auto waitFunction = sourceEntry.waitFunction;
        auto asyncIter = mAsyncProcesses.at(sourceId).find(resourceId);

        if(asyncIter != mAsyncProcesses.at(sourceId).end())
            waitFunction(asyncIter->second);

        {
            std::lock_guard<std::recursive_mutex> lock(mResourceMutex);
            mResources.at(sourceId).erase(resourceId);
            mAsyncProcesses.at(sourceId).erase(resourceId);
        }
#else
        mResources.at(sourceId).erase(resourceId);
#endif
    }

    inline void ResourceProvider::markAllUnused(const std::string& sourceId)
    {
        const auto& sourceEntry = toSourceEntry(sourceId);

#ifndef REX_DISABLE_ASYNC
        waitForSourceAsync(sourceId);

        {
            std::lock_guard<std::recursive_mutex> lock(mResourceMutex);
            mResources.at(sourceId).clear();
            mAsyncProcesses.at(sourceId).clear();
        }
#else
            mResources.at(sourceId).clear();
#endif
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
#ifndef REX_DISABLE_ASYNC
        auto sourceIterator = mAsyncProcesses.find(sourceId);

        if(sourceIterator != mAsyncProcesses.end())
            return sourceIterator->second.count(resourceId);

#endif
        return false;
    }

    template <typename ResourceType>
    const ResourceType& ResourceProvider::loadResource(const std::string& sourceId, const std::string& resourceId) const
    {
        const auto& sourceEntry = toSourceEntry(sourceId);

        try
        {
            auto loadFunction = sourceEntry.loadingFunction.get<LoadingFunction<ResourceType>>();
            auto resource = loadFunction(sourceEntry.source, resourceId);

#ifndef REX_DISABLE_ASYNC
            {
                std::lock_guard<std::recursive_mutex> lock(mResourceMutex);
                auto emplaced = mResources.at(sourceId).emplace(resourceId, std::move(resource));

                mAsyncProcesses.at(sourceId).erase(resourceId);

                return emplaced.first->second.template get<ResourceType>();
            }
#else
			auto emplaced = mResources.at(sourceId).emplace(resourceId, std::move(resource));
			return emplaced.first->second.template get<ResourceType>();
#endif
        }
        catch(const std::exception& exception)
        {
            throw InvalidResourceException(exception.what());
        }
    }

    inline void ResourceProvider::waitForSourceAsync(const std::string& sourceId) const
    {
#ifndef REX_DISABLE_ASYNC
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            auto waitFunction = sourceIterator->second.waitFunction;

            for(auto& asyncIter : mAsyncProcesses.at(sourceId))
                waitFunction(asyncIter.second);
        }
#endif
    }

    inline const ResourceProvider::SourceEntry& ResourceProvider::toSourceEntry(const std::string& sourceId) const
    {
        auto sourceIterator = mSources.find(sourceId);

        if(sourceIterator != mSources.end())
        {
            return sourceIterator->second;
        }
        else
            throw InvalidSourceException("trying to access source id " + sourceId + " which doesn't exist");
    }
}
