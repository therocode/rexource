#include <catch.hpp>
#include <rex/filelister.hpp>

SCENARIO("File listers cannot be created from entries that are not folders")
{
    GIVEN("a file lister")
    {
        WHEN("it is created from a path describing a file")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(rex::FileLister("tests/data/folders/file1", rex::FileLister::FILES), rex::InvalidFileException);
            }
        }

        WHEN("it is created from a path describing neither a file nor a folder")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(rex::FileLister("tests/data/floldrers", rex::FileLister::FILES), rex::InvalidFileException);
            }
        }
    }
}

SCENARIO("File listers can be used to list all files in a folder recursively")
{
    GIVEN("a file lister with a given directory path with FILES mode")
    {
        rex::FileLister fileLister(std::string("tests/data/folders"), rex::FileLister::FILES);

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
}

SCENARIO("File listers can be used to list all folders in a folder recursively")
{
    GIVEN("a file lister with a given directory path with FOLDERS mode")
    {
        rex::FileLister fileLister(std::string("tests/data/folders"), rex::FileLister::FOLDERS);

        WHEN("the lister is asked for a file list")
        {
            std::vector<rex::Path> fileList = fileLister.list();

            THEN("the list contains all the contained folders - excluding files")
            {
                REQUIRE(fileList.size() == 6);

                std::set<std::string> fileSet(fileList.begin(), fileList.end());

                CHECK(fileSet.count("tests/data/folders/folder1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2/folder1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3/folder1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2/folder1/folder1") != 0);
            }
        }
    }
}

SCENARIO("File listers can be used to list all files and folders in a folder recursively")
{
    GIVEN("a file lister with a given directory path with FOLDERS mode")
    {
        rex::FileLister fileLister(std::string("tests/data/folders"), rex::FileLister::ALL);

        WHEN("the lister is asked for a file list")
        {
            std::vector<rex::Path> fileList = fileLister.list();

            THEN("the list contains all the contained folders - excluding files")
            {
                REQUIRE(fileList.size() == 14);

                std::set<std::string> fileSet(fileList.begin(), fileList.end());

                CHECK(fileSet.count("tests/data/folders/folder1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2/folder1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder3/folder1") != 0);
                CHECK(fileSet.count("tests/data/folders/folder2/folder1/folder1") != 0);
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
}
