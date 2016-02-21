#include <catch.hpp>
#include <rex/resourceprovider.hpp>
#include <rex/progresstracker.hpp>

#include "helpers/treefilesource.hpp"

#ifndef REX_DISABLE_ASYNC
SCENARIO("ProgressTracker can tell the current progress of an async loading process")
{
    GIVEN("a resource provider with a source set up")
    {
        rex::ResourceProvider provider;
        provider.addSource("trees", TreeFileSource("tests/data/trees"));

        WHEN("a ProgressTracker is created out of an async loading process from the ResourceProvider")
        {
            rex::ProgressTracker<Tree> tracker(provider.asyncGetAll<Tree>("trees"));

            THEN("the tracker reports the correct total amount")
            {
                CHECK(tracker.total() == 1000);
                CHECK(tracker.status().total() == 1000);
            }

            THEN("constantly polling the tracker gives reasonable values")
            {
                int32_t lastDone = -1;
                int32_t lastWaiting = 1001;
                float lastDoneRatio = -1.0f;
                float lastWaitingRatio = 2.0f;

                while(lastDoneRatio < 1.0f)
                {
                    rex::ProgressTracker<Tree>::Status status = tracker.status();

                    int32_t newDone = status.done();
                    int32_t newWaiting = status.waiting();
                    int32_t failed = status.failed();

                    float newDoneRatio = status.doneRatio();
                    float newWaitingRatio = status.waitingRatio();
                    float failedRatio = status.failedRatio();

                    CHECK(newDone >= lastDone);
                    CHECK(newWaiting <= lastWaiting);
                    CHECK(failed == 0);

                    CHECK(newDoneRatio >= lastDoneRatio);
                    CHECK(newWaitingRatio <= lastWaitingRatio);
                    CHECK(failedRatio == 0.0f);

                    lastDone = newDone;
                    lastWaiting = newWaiting;

                    lastDoneRatio = newDoneRatio;
                    lastWaitingRatio = newWaitingRatio;
                }

                rex::ProgressTracker<Tree>::Status status = tracker.status();

                CHECK(lastDone == 1000);
                CHECK(lastWaiting == 0);
                CHECK(status.failed() == 0);
                CHECK(lastDoneRatio == 1.0f);
                CHECK(lastWaitingRatio == 0.0f);
                CHECK(status.failedRatio() == 0.0f);
            }
        }

        WHEN("a ProgressTracker is created out of an async loading process from the ResourceProvider, containing some invalid entries")
        {
            rex::ProgressTracker<Tree> tracker(provider.asyncGet<Tree>("trees", {"tree1", "tree2", "blah", "tree3", "bloh", "blih", "tree4", "bleh"}));

            THEN("the tracker reports the correct total amount")
            {
                CHECK(tracker.total() == 8);
                CHECK(tracker.status().total() == 8);
            }

            THEN("constantly polling the tracker gives reasonable values")
            {
                int32_t lastDone = -1;
                int32_t lastWaiting = 1001;
                int32_t lastFailed = -1;
                float lastDoneRatio = -1.0f;
                float lastWaitingRatio = 2.0f;
                float lastFailedRatio = -1.0f;

                while(lastWaitingRatio > 0.0f)
                {
                    rex::ProgressTracker<Tree>::Status status = tracker.status();

                    int32_t newDone = status.done();
                    int32_t newWaiting = status.waiting();
                    int32_t newFailed = status.failed();

                    float newDoneRatio = status.doneRatio();
                    float newFailedRatio = status.failedRatio();
                    float newWaitingRatio = status.waitingRatio();

                    CHECK(newDone >= lastDone);
                    CHECK(newWaiting <= lastWaiting);
                    CHECK(newFailed >= lastFailed);

                    CHECK(newDoneRatio >= lastDoneRatio);
                    CHECK(newWaitingRatio <= lastWaitingRatio);
                    CHECK(newFailedRatio >= lastFailedRatio);

                    lastDone = newDone;
                    lastWaiting = newWaiting;
                    lastFailed = newFailed;

                    lastDoneRatio = newDoneRatio;
                    lastWaitingRatio = newWaitingRatio;
                    lastFailedRatio = newFailedRatio;
                }

                CHECK(lastDone == 4);
                CHECK(lastWaiting == 0);
                CHECK(lastFailed == 4);
                CHECK(lastDoneRatio == Approx(0.5f));
                CHECK(lastWaitingRatio == 0.0f);
                CHECK(lastFailedRatio == Approx(0.5f));
            }
        }
    }
}
#endif
