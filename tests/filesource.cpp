#include <catch.hpp>
#include "helpers/treefileloader.hpp"

SCENARIO("File loaders can be used to load files from directory trees with regex filtering")
{
    GIVEN("a file loader which is set to read all files in a directory")
    {
        //TreeFileLoader("data/trees", ".*");

        WHEN("all files are listed by the source")
        {
            THEN("the list contains the expected result")
            {

            }
        }
    }
}
