#include <catch.hpp>
#include "helpers/treefilesource.hpp"

SCENARIO("File sources set to a folder with a regex will find files recursively with the regex as a filter")
{
    GIVEN("a file source which is set to read all files in a directory")
    {
        TreeFileSource treeSource("tests/data/trees", std::regex(".*"));

        WHEN("all contained resurces are listed by the source")
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

    GIVEN("a file source which is set to read all files with a number in the 500s in a directory")
    {
        TreeFileSource treeSource("tests/data/trees", std::regex(".*5\\d\\d.*"));

        WHEN("all contained resurces are listed by the source")
        {
            auto trees = treeSource.list();

            THEN("the list contains the expected result")
            {
                REQUIRE(trees.size() == 100);

                std::set<std::string> paths(trees.begin(), trees.end());

                for(int32_t i = 500; i < 600; ++i)
                {
                    std::string expectedId = "tree" + std::to_string(i);

                    CHECK(paths.count(expectedId) != 0);
                }
            }
        }
    }

    GIVEN("a file source which is not given a regex")
    {
        TreeFileSource treeSource("tests/data/trees");

        WHEN("all contained resurces are listed by the source")
        {
            auto trees = treeSource.list();

            THEN("the list contains all the files")
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

    GIVEN("a file source which is set to read all files within a directory that doesn't exist")
    {
        WHEN("all contained resurces are listed by the source")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(TreeFileSource("tests/data/tres", std::regex(".*")), rex::InvalidFileException);
            }
        }
    }
}

SCENARIO("File sources name resources differently based on the Naming flag. This affects ambiguity of resources")
{
    GIVEN("")
    {
        WHEN("a file source is used to load a folder with entirely unique file names")
        {
            THEN("it doesn't throw with Naming::NO_EXT")
            {
                CHECK_NOTHROW(TreeFileSource("tests/data/unique", std::regex(".*"), rex::Naming::NO_EXT));
            }
            THEN("it doesn't throw with Naming::FILE_NAME")
            {
                CHECK_NOTHROW(TreeFileSource("tests/data/unique", std::regex(".*"), rex::Naming::FILE_NAME));
            }
            THEN("it doesn't throw with Naming::PATH")
            {
                CHECK_NOTHROW(TreeFileSource("tests/data/unique", std::regex(".*"), rex::Naming::PATH));
            }
        }

        WHEN("a file source is used to load a folder with file names that collide with file extensions")
        {
            THEN("it throws with Naming::NO_EXT")
            {
                CHECK_THROWS_AS(TreeFileSource("tests/data/collide_ext", std::regex(".*"), rex::Naming::NO_EXT), rex::AmbiguousNameException);
            }
            THEN("it throws with Naming::FILE_NAME")
            {
                CHECK_THROWS_AS(TreeFileSource("tests/data/collide_ext", std::regex(".*"), rex::Naming::FILE_NAME), rex::AmbiguousNameException);
            }
            THEN("it doesn't throw with Naming::PATH")
            {
                CHECK_NOTHROW(TreeFileSource("tests/data/collide_ext", std::regex(".*"), rex::Naming::PATH));
            }
        }

        WHEN("a file source is used to load a folder with file names that collide without file extensions")
        {
            THEN("it throws with Naming::NO_EXT")
            {
                CHECK_THROWS_AS(TreeFileSource("tests/data/collide_no_ext", std::regex(".*"), rex::Naming::NO_EXT), rex::AmbiguousNameException);
            }
            THEN("it doesn't throw with Naming::FILE_NAME")
            {
                CHECK_NOTHROW(TreeFileSource("tests/data/collide_no_ext", std::regex(".*"), rex::Naming::FILE_NAME));
            }
            THEN("it doesn't throw with Naming::PATH")
            {
                CHECK_NOTHROW(TreeFileSource("tests/data/collide_no_ext", std::regex(".*"), rex::Naming::PATH));
            }
        }
    }
}

SCENARIO("a file source can be used to access the resources it represents")
{
    GIVEN("a file source setup to a directory with resources")
    {
        TreeFileSource treeSource("tests/data/trees");

        WHEN("existing resources are accessed")
        {
            const Tree& tree1 = treeSource.load("tree1");
            const Tree& tree2 = treeSource.load("tree2");

            THEN("the resources have correct values")
            {
                CHECK(tree1.leafType == "gigantic");
                CHECK(tree1.branchingFactor == Approx(65.7f));
                CHECK(tree1.barkType == "naked");
                CHECK(tree1.height == Approx(5.2f));

                CHECK(tree2.leafType == "wide");
                CHECK(tree2.branchingFactor == Approx(78.1f));
                CHECK(tree2.barkType == "paper");
                CHECK(tree2.height == Approx(43.2f));
            }
        }
    }
}
