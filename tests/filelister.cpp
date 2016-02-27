#include <catch.hpp>
#include <rex/filelister.hpp>

SCENARIO("File listers can be used to list all files in a folder recursively")
{
    GIVEN("a file lister with a given directory path")
    {
        rex::FileLister fileLister(std::string("tests/data/folders"));

        WHEN("the lister is asked for a file list")
        {
            std::vector<rex::Path> fileList = fileLister.list();

            THEN("the list contains all the contained files - excluding folders")
            {
                REQUIRE(fileList.size() == 8);

                std::set<std::string> fileSet(fileList.begin(), fileList.end());

                CHECK(fileSet.count("tests/data/folders/file1") != 0);
                CHECK(fileSet.count("tests/data/folders/file2") != 0);
                CHECK(fileSet.count("tests/data/folders/folder1/file1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2/folder1/folder1/file1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2/folder1/folder1/file2") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3/folder1/file1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3/folder1/file2") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3/file1") != 0);
            }
        }
    }

    GIVEN("a file lister")
    {
        WHEN("it is created from a path describing a file")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(rex::FileLister("tests/data/folders/file1"), rex::InvalidFileException);
            }
        }

        WHEN("it is created from a path describing neither a file nor a folder")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(rex::FileLister("tests/data/floldrers"), rex::InvalidFileException);
            }
        }
    }
}
