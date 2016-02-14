#include <catch.hpp>
#include "helpers/treefilesource.hpp"

SCENARIO("File loaders set to a folder with a regex will find files recursively with the regex as a filter")
{
    GIVEN("a file loader which is set to read all files in a directory")
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

    GIVEN("a file loader which is set to read all files with a number in the 500s in a directory")
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

    GIVEN("a file loader which is not given a regex")
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

    GIVEN("a file loader which is set to read all files within a directory that doesn't exist")
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

SCENARIO("File loaders name resources differently based on the Naming flag. This affects ambiguity of resources")
{
    GIVEN("")
    {
        WHEN("a file loader is used to load a folder with entirely unique file names")
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

        WHEN("a file loader is used to load a folder with file names that collide with file extensions")
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

        WHEN("a file loader is used to load a folder with file names that collide without file extensions")
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
