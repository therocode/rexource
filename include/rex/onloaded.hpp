#pragma once
#include <vector>
#include <rex/asyncresourceview.hpp>

namespace rex
{
    template <typename ResourceType>
    class OnLoaded
    {
        public:
            OnLoaded(std::vector<AsyncResourceView<ResourceType>> toTrack, std::function<void(const std::string&, const ResourceType&)> callback);
            void poll();
        private:
            std::vector<AsyncResourceView<ResourceType>> mToTrack;
            std::vector<bool> mDoneEntries;
            std::function<void(const std::string&, const ResourceType&)> mCallback;
    };

    template <typename ResourceType>
    OnLoaded<ResourceType>::OnLoaded(std::vector<AsyncResourceView<ResourceType>> toTrack, std::function<void(const std::string&, const ResourceType&)> callback):
        mToTrack(std::move(toTrack)),
        mDoneEntries(mToTrack.size(), false),
        mCallback(std::move(callback))
    {
    }

    template <typename ResourceType>
    void OnLoaded<ResourceType>::poll()
    {
        for(size_t i = 0; i < mDoneEntries.size(); ++i)
        {
            bool entryDone = mDoneEntries[i];

            if(!entryDone)
            {
                auto& entry = mToTrack[i];
                if(entry.future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                {
                    try
                    {
                        mCallback(entry.identifier, entry.future.get());
                    }
                    catch(...)
                    {
                    }

                    mDoneEntries[i] = true;
                }
            }
        }
    }
}
