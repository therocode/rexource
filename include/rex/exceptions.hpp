#pragma once
#include <stdexcept>

namespace rex
{
    class InvalidSourceException : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class InvalidResourceException : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class InvalidFileException : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };
}
