#pragma once
#include <vector>
#include <rex/thero.hpp>
#include <rex/asyncresourceview.hpp>

namespace rex
{
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

            template <typename ResourceType>
            ProgressTracker(std::vector<AsyncResourceView<ResourceType>> toTrack);
            int32_t total() const;
            Status status() const;
        private:
            int32_t mTotal;
            th::Any mToTrack;
            Status (*mStatusGetter)(const th::Any&);
    };

    inline ProgressTracker::Status::Status(int32_t waiting, int32_t done, int32_t failed):
        mTotal(waiting + done + failed),
        mWaiting(waiting),
        mDone(done),
        mFailed(failed),
        mWaitingRatio(toRatio(waiting)),
        mDoneRatio(toRatio(done)),
        mFailedRatio(toRatio(failed))
    {
    }

    inline int32_t ProgressTracker::Status::total() const
    {
        return mTotal;
    }

    inline int32_t ProgressTracker::Status::waiting() const
    {
        return mWaiting;
    }

    inline int32_t ProgressTracker::Status::done() const
    {
        return mDone;
    }

    inline int32_t ProgressTracker::Status::failed() const
    {
        return mFailed;
    }

    inline float ProgressTracker::Status::waitingRatio() const
    {
        return mWaitingRatio;
    }

    inline float ProgressTracker::Status::doneRatio() const
    {
        return mDoneRatio;
    }

    inline float ProgressTracker::Status::failedRatio() const
    {
        return mFailedRatio;
    }

    inline float ProgressTracker::Status::toRatio(int32_t amount) const
    {
        if(amount == 0)
            return 0.0f;
        else if(amount == mTotal)
            return 1.0f;
        else
            return static_cast<float>(amount) / static_cast<float>(mTotal);
    }

    template <typename ResourceType>
    ProgressTracker::ProgressTracker(std::vector<AsyncResourceView<ResourceType>> toTrack):
        mTotal(toTrack.size()),
        mToTrack(std::move(toTrack))
    {
        mStatusGetter = [] (const th::Any& trackedAny)
        {
            const auto& tracked = trackedAny.get<std::vector<AsyncResourceView<ResourceType>>>();

            int32_t waiting = 0;
            int32_t done = 0;
            int32_t failed = 0;

            for(const auto& view : tracked)
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

            return Status(
                waiting,
                done,
                failed
            );
        };
    }

    inline int32_t ProgressTracker::total() const
    {
        return mTotal;
    }

    inline ProgressTracker::Status ProgressTracker::status() const
    {
        return mStatusGetter(mToTrack);
    }
}
