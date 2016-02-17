#include <catch.hpp>
#include <rex/resourceprovider.hpp>
#include <rex/progresstracker.hpp>
#include <rex/onloaded.hpp>

#include "helpers/treefilesource.hpp"

SCENARIO("OnLoaded can be used to make callback functions execute on successfully loaded objects")
{
    GIVEN("a resource provider with a source set up")
    {
        rex::ResourceProvider provider;
        provider.addSource("trees", TreeFileSource("tests/data/trees"));

        WHEN("an OnLoaded is created out of an async loading process from the ResourceProvider and given a callback function")
        {
            std::vector<rex::AsyncResourceView<Tree>> asyncResources = provider.asyncGetAll<Tree>("trees");

            std::set<int32_t> loadedTreeIds;
            rex::OnLoaded<Tree> onLoaded(asyncResources, [&loadedTreeIds] (const std::string& identifier, const Tree& loadedResource)
            {
                size_t numberStartAt = identifier.find_first_of("1234567890");
                std::string startsWithNumber = identifier.substr(numberStartAt);
                int32_t number = std::stoi(startsWithNumber);

                loadedTreeIds.emplace(number);
            });

            THEN("eventually, the callback is run for every object")
            {
                auto tracker = rex::ProgressTracker<Tree>(asyncResources);

                size_t lastSize = loadedTreeIds.size();

                while(tracker.status().waiting > 0)
                {
                    onLoaded.poll();

                    size_t newSize = loadedTreeIds.size();
                    CHECK(newSize >= lastSize);
                    lastSize = newSize;
                }
                onLoaded.poll();

                CHECK(loadedTreeIds.size() == 1000);

                for(int32_t i = 0; i < 1000; ++i)
                    CHECK(loadedTreeIds.count(i) != 0);
            }
        }

        WHEN("an OnLoaded is created out of an async loading process with invalid entries from the ResourceProvider and given a callback function")
        {
            std::vector<rex::AsyncResourceView<Tree>> asyncResources = provider.asyncGet<Tree>("trees", {"tree1", "asdf", "tree2", "blah", "tree3", "gropp"});

            std::set<int32_t> loadedTreeIds;
            rex::OnLoaded<Tree> onLoaded(asyncResources, [&loadedTreeIds] (const std::string& identifier, const Tree& loadedResource)
            {
                size_t numberStartAt = identifier.find_first_of("1234567890");
                std::string startsWithNumber = identifier.substr(numberStartAt);
                int32_t number = std::stoi(startsWithNumber);

                loadedTreeIds.emplace(number);
            });

            THEN("eventually, the callback is run for every successful object only")
            {
                auto tracker = rex::ProgressTracker<Tree>(asyncResources);

                size_t lastSize = loadedTreeIds.size();

                while(tracker.status().waiting > 0)
                {
                    onLoaded.poll();

                    size_t newSize = loadedTreeIds.size();
                    CHECK(newSize >= lastSize);
                    lastSize = newSize;
                }
                onLoaded.poll();

                CHECK(loadedTreeIds.size() == 3);

                CHECK(loadedTreeIds.count(1) != 0);
                CHECK(loadedTreeIds.count(2) != 0);
                CHECK(loadedTreeIds.count(3) != 0);
            }
        }
    }
}
