#include <catch.hpp>
#include <rex/path.hpp>

SCENARIO("Paths represent a file path")
{
    GIVEN("A path setup to a normal file")
    {
        rex::Path path("data/folder/file.ext");

        WHEN("a string is accessed and it is cast to a string implicitly")
        {
            std::string explicitConvert = path.str();
            std::string implicitConvert = path;

            THEN("the strings are the same")
            {
                CHECK(explicitConvert == implicitConvert);
            }
        }

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/file.ext");
                CHECK(fileName == "file.ext");
                CHECK(extension == "ext");
                CHECK(stem == "file");
            }
        }
    }

    GIVEN("A path with backslashes")
    {
        rex::Path path("data\\folder\\file.ext");

        WHEN("path is accessed")
        {
            std::string fullPath = path;

            THEN("the backslashes are convered into frontslashes")
            {
                CHECK(fullPath == "data/folder/file.ext");
            }
        }
    }

    GIVEN("A path ending in a slash")
    {
        rex::Path path("data/folder/");

        WHEN("path is accessed")
        {
            std::string fullPath = path;

            THEN("the slash is stripped")
            {
                CHECK(fullPath == "data/folder");
            }
        }
    }

    GIVEN("A path setup to a file without extension")
    {
        rex::Path path("data/folder");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder");
                CHECK(fileName == "folder");
                CHECK(extension == "");
                CHECK(stem == "folder");
            }
        }
    }

    GIVEN("A path setup to a file with an extension and a period in its name")
    {
        rex::Path path("data/folder/fi.le.ext");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/fi.le.ext");
                CHECK(fileName == "fi.le.ext");
                CHECK(extension == "ext");
                CHECK(stem == "fi.le");
            }
        }
    }

    GIVEN("A path setup to a file with an empty extension")
    {
        rex::Path path("data/folder/file.");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/file.");
                CHECK(fileName == "file.");
                CHECK(extension == "");
                CHECK(stem == "file");
            }
        }
    }

    GIVEN("A path setup to a file with an empty filename")
    {
        rex::Path path("data/folder/.ext");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/.ext");
                CHECK(fileName == ".ext");
                CHECK(extension == "ext");
                CHECK(stem == "");
            }
        }
    }

    GIVEN("A path setup to a file with '.' as the name")
    {
        rex::Path path("data/folder/.");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/.");
                CHECK(fileName == ".");
                CHECK(extension == "");
                CHECK(stem == ".");
            }
        }
    }

    GIVEN("A path setup to a file with '..' as the name")
    {
        rex::Path path("data/folder/..");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/..");
                CHECK(fileName == "..");
                CHECK(extension == "");
                CHECK(stem == "..");
            }
        }
    }

    GIVEN("A path setup to a file with '...' as the name")
    {
        rex::Path path("data/folder/...");

        WHEN("path data is accessed")
        {
            std::string fullPath = path;
            std::string fileName = path.fileName();
            std::string extension = path.extension();
            std::string stem = path.stem();

            THEN("correct data is given")
            {
                CHECK(fullPath == "data/folder/...");
                CHECK(fileName == "...");
                CHECK(extension == "");
                CHECK(stem == "...");
            }
        }
    }
}
