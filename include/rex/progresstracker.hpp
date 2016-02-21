#pragma once
#include <vector>
#include <rex/asyncresourceview.hpp>

namespace rex
{
    template <typename ResourceType>
    class ProgressTracker
    {
        public:
            class Status
            {
                public:
                    Status(int32_t waiting, int32_t done, int32_t failed);
                    int32_t total() const;
                    int32_t waiting() const;
                    int32_t done() const;
                    int32_t failed() const;
                    float waitingRatio() const;
                    float doneRatio() const;
                    float failedRatio() const;
                private:
                    float toRatio(int32_t amount) const;
                    int32_t mTotal;
                    int32_t mWaiting;
                    int32_t mDone;
                    int32_t mFailed;
                    float mWaitingRatio;
                    float mDoneRatio;
                    float mFailedRatio;
            };

            ProgressTracker(std::vector<AsyncResourceView<ResourceType>> toTrack);
            int32_t total() const;
            Status status() const;
        private:
            void updateValues() const;
            std::vector<AsyncResourceView<ResourceType>> mToTrack;
            mutable int32_t mDoneCount;
            mutable int32_t mWaitingCount;
            mutable int32_t mFailedCount;
    };

    template <typename ResourceType>
    ProgressTracker<ResourceType>::Status::Status(int32_t waiting, int32_t done, int32_t failed):
        mTotal(waiting + done + failed),
        mWaiting(waiting),
        mDone(done),
        mFailed(failed),
        mWaitingRatio(toRatio(waiting)),
        mDoneRatio(toRatio(done)),
        mFailedRatio(toRatio(failed))
    {
    }

    template <typename ResourceType>
    int32_t ProgressTracker<ResourceType>::Status::total() const
    {
        return mTotal;
    }

    template <typename ResourceType>
    int32_t ProgressTracker<ResourceType>::Status::waiting() const
    {
        return mWaiting;
    }

    template <typename ResourceType>
    int32_t ProgressTracker<ResourceType>::Status::done() const
    {
        return mDone;
    }

    template <typename ResourceType>
    int32_t ProgressTracker<ResourceType>::Status::failed() const
    {
        return mFailed;
    }

    template <typename ResourceType>
    float ProgressTracker<ResourceType>::Status::waitingRatio() const
    {
        return mWaitingRatio;
    }

    template <typename ResourceType>
    float ProgressTracker<ResourceType>::Status::doneRatio() const
    {
        return mDoneRatio;
    }

    template <typename ResourceType>
    float ProgressTracker<ResourceType>::Status::failedRatio() const
    {
        return mFailedRatio;
    }

    template <typename ResourceType>
    float ProgressTracker<ResourceType>::Status::toRatio(int32_t amount) const
    {
        if(amount == 0)
            return 0.0f;
        else if(amount == mTotal)
            return 1.0f;
        else
            return static_cast<float>(amount) / static_cast<float>(mTotal);
    }

    template <typename ResourceType>
    ProgressTracker<ResourceType>::ProgressTracker(std::vector<AsyncResourceView<ResourceType>> toTrack):
        mToTrack(std::move(toTrack)),
        mDoneCount(0),
        mWaitingCount(0),
        mFailedCount(0)
    {
    }

    template <typename ResourceType>
    int32_t ProgressTracker<ResourceType>::total() const
    {
        return static_cast<int32_t>(mToTrack.size());
    }

    template <typename ResourceType>
    typename ProgressTracker<ResourceType>::Status ProgressTracker<ResourceType>::status() const
    {
        updateValues();

        return Status(
            mWaitingCount,
            mDoneCount,
            mFailedCount
        );
    }

    template <typename ResourceType>
    void ProgressTracker<ResourceType>::updateValues() const
    {
        int32_t waiting = 0;
        int32_t done = 0;
        int32_t failed = 0;


        for(const auto& view : mToTrack)
        {
            auto futureStatus = view.future.wait_for(std::chrono::milliseconds(0));
            if(futureStatus == std::future_status::timeout)
                ++waiting;
            else if(futureStatus == std::future_status::ready)
            {
                try
                {
                    view.future.get();
                    ++done;
                }
                catch(...)
                {
                    ++failed;
                }
            }
        }

        mWaitingCount = waiting;
        mDoneCount = done;
        mFailedCount = failed;
    }
}
