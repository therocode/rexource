#cmake ver req
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Select if you want to build Debug or Release" FORCE )
endif()

set(BUNDLE_THERO TRUE CACHE BOOL "Will include the needed files from the thero support library. If not set, it must be installed separately")
set(BUNDLE_TINYDIR TRUE CACHE BOOL "Will include the needed files from the tinydir file library. If not set, it must be installed separately")
set(BUNDLE_JSON TRUE CACHE BOOL "Will include the needed files from the nlohmann json library. If not set, it must be installed separately")
set(INCLUDE_SFML_LOADERS FALSE CACHE BOOL "Will include the resource loaders for SFML resources")
set(INCLUDE_FEATHERKIT_LOADERS FALSE CACHE BOOL "Will include the resource loaders for Feather Kit resources")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/")

set(BUILD_TESTS TRUE CACHE BOOL "build the tests")
set(DISABLE_ASYNC FALSE CACHE BOOL "builds the tests without asynchronous components")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  #for static libs 
 
set(project_name rexources)
project(${project_name} CXX)

add_definitions("-DREX_DISABLE_CONFIG")

if(DISABLE_ASYNC)
    add_definitions("-DREX_DISABLE_ASYNC")
endif()

set(header_files
    include/rex/assert.hpp
    include/rex/asyncresourceview.hpp
    include/rex/config.hpp
    include/rex/exceptions.hpp
    include/rex/filesource.hpp
    include/rex/filelister.hpp
    include/rex/json.hpp
    include/rex/onloaded.hpp
    include/rex/progresstracker.hpp
    include/rex/path.hpp
    include/rex/resourceprovider.hpp
    include/rex/resourceview.hpp
    include/rex/sfml.hpp
    include/rex/sourceview.hpp
    include/rex/thero.hpp
    include/rex/threadpool.hpp
    include/rex/tinydir.hpp
)

if(INCLUDE_SFML_LOADERS)
    list(APPEND header_files
        include/rex/sfml/texturefilesource.hpp
        include/rex/sfml/imagefilesource.hpp
        include/rex/sfml/soundbufferfilesource.hpp
        include/rex/sfml/fontfilesource.hpp
    )
endif()

include_directories("include")
include_directories("thero/include")
include_directories("tinydir")
include_directories("json/src")

#tests
if(BUILD_TESTS)
    include_directories(tests)

    set(test_sources
        "tests/main.cpp"
        "tests/helpers/peoplesource.cpp"
        "tests/helpers/toolsource.cpp"
        "tests/filesource.cpp"
        "tests/filelister.cpp"
        "tests/onloaded.cpp"
        "tests/path.cpp"
        "tests/progresstracker.cpp"
        "tests/resourceprovider.cpp"
        "tests/threadpool.cpp"
    )

    add_executable(tests ${test_sources})

    set_property(TARGET tests PROPERTY CXX_STANDARD 11)
    set_property(TARGET tests PROPERTY CXX_STANDARD_REQUIRED ON)

    find_package(Threads)

    target_link_libraries(tests ${CMAKE_THREAD_LIBS_INIT})
endif()

#installation
if(WIN32)
    set(REX_MISC_DIR .)
    set(WINFIX SYSTEM)
else()
    set(REX_MISC_DIR share/rexources)
endif()

install(
    DIRECTORY include
    DESTINATION .
    )

#install(
#    TARGETS ${project_name}
#    RUNTIME DESTINATION bin COMPONENT bin
#    LIBRARY DESTINATION lib COMPONENT bin
#    ARCHIVE DESTINATION lib COMPONENT dev
#    )

install(
    FILES README.md LICENSE
    DESTINATION ${REX_MISC_DIR}
    )

#thero
if(BUNDLE_THERO)
    install(
        FILES thero/include/thero/optional.hpp thero/include/thero/any.hpp
        DESTINATION ./include/rex/thero/
        )
    install(
        FILES config/bundlethero.hpp
        DESTINATION ./include/rex/config
        RENAME thero.hpp
        )
else()
    install(
        FILES config/nobundlethero.hpp
        DESTINATION ./include/rex/config
        RENAME thero.hpp
        )
endif()

#tinydir
if(BUNDLE_TINYDIR)
    install(
        FILES tinydir/tinydir.h
        DESTINATION ./include/rex/tinydir/
        )
    install(
        FILES config/bundletinydir.hpp
        DESTINATION ./include/rex/config
        RENAME tinydir.hpp
        )
else()
    install(
        FILES config/nobundletinydir.hpp
        DESTINATION ./include/rex/config
        RENAME tinydir.hpp
        )
endif()

#json
if(BUNDLE_JSON)
    install(
        FILES json/src/json.hpp
        DESTINATION ./include/rex/json
        )
    install(
        FILES config/bundlejson.hpp
        DESTINATION ./include/rex/config
        RENAME json.hpp
        )
else()
    install(
        FILES config/nobundlejson.hpp
        DESTINATION ./include/rex/config
        RENAME json.hpp
        )
endif()
