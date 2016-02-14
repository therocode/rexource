#include <catch.hpp>
#include "helpers/treefilesource.hpp"

SCENARIO("File loaders can be used to load files from directory trees with regex filtering")
{
    GIVEN("a file loader which is set to read all files in a directory")
    {
        TreeFileSource treeSource("data/trees", ".*");

        WHEN("all files are listed by the source")
        {
            auto trees = treeSource.list();

            THEN("the list contains the expected result")
            {
                REQUIRE(trees.size() == 1000);

                std::set<std::string> paths(trees.begin(), trees.end());

                for(int32_t i = 0; i < 1000; ++i)
                {
                    std::string expectedId = "tree" + std::to_string(i);

                    CHECK(paths.count(expectedId) != 0);
                }
            }
        }
    }
}
