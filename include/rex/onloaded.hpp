#pragma once
#include <vector>
#include <rex/asyncresourceview.hpp>

namespace rex
{
    class OnLoaded
    {
        public:
            template <typename ResourceType>
            OnLoaded(std::vector<AsyncResourceView<ResourceType>> toTrack, std::function<void(const std::string&, const decltype(toTrack[0].future.get())&)> callback);
            void poll();
        private:
            std::vector<bool> mDoneEntries;
            th::Any mToTrack;
            th::Any mCallback;

            bool (*mEntryReady)(const th::Any& trackedAny, size_t index);
            void (*mExecuteCallback)(const th::Any& trackedAny, size_t index, const th::Any& callbackAny);
    };

    template <typename ResourceType>
    OnLoaded::OnLoaded(std::vector<AsyncResourceView<ResourceType>> toTrack, std::function<void(const std::string&, const decltype(toTrack[0].future.get())&)> callback):
        mDoneEntries(toTrack.size(), false),
        mToTrack(std::move(toTrack)),
        mCallback(std::move(callback))
    {
        mEntryReady = [] (const th::Any& trackedAny, size_t index)
        {
            const auto& tracked = trackedAny.get<std::vector<AsyncResourceView<ResourceType>>>();

            return tracked[index].future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        };

        mExecuteCallback = [] (const th::Any& trackedAny, size_t index, const th::Any& callbackAny)
        {
            const auto& tracked = trackedAny.get<std::vector<AsyncResourceView<ResourceType>>>();
            const auto& callback = callbackAny.get<std::function<void(const std::string&, const ResourceType&)>>();

            const ResourceType* resource = nullptr;

            try
            {
                resource = &tracked[index].future.get();
            }
            catch(...)
            {
            }

            if(resource)
            {
                callback(tracked[index].identifier, *resource);
            }
        };
    }

    inline void OnLoaded::poll()
    {
        for(size_t i = 0; i < mDoneEntries.size(); ++i)
        {
            bool entryDone = mDoneEntries[i];

            if(!entryDone)
            {
                if(mEntryReady(mToTrack, i))
                {
                    mDoneEntries[i] = true;

                    mExecuteCallback(mToTrack, i, mCallback);
                }
            }
        }
    }
}
