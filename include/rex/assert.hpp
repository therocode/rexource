#pragma once
#include <rex/config.hpp>
#include <iostream>
#include <csignal>

#ifndef SIGTRAP
#define SIGTRAP 5
#endif

#ifndef EMSCRIPTEN
#define REX_HALT raise(SIGTRAP)
#else
#define REX_HALT exit(1)
#endif

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

#ifndef NDEBUG
#   define REX_ASSERT(condition, message) \
    do\
    { \
        if(!(condition))\
        { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
            << " function " << __func__ << " line " << __LINE__ << ": " << message << std::endl; \
            REX_HALT; \
        } \
    } while (false)
#else
#   define REX_ASSERT(condition, message) do { } while (false)
#endif
