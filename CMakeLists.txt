#cmake ver req
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Select if you want to build Debug or Release" FORCE )
endif()

set(BUNDLE_THERO FALSE CACHE BOOL "Will include the needed files from the thero support library. If not set, thero must be installed separately")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/")

set(BUILD_TESTS TRUE CACHE BOOL "build the tests")
#set(BUILD_EXAMPLES TRUE CACHE BOOL "build the examples")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  #for static libs 
 
set(project_name rexources)
project(${project_name} CXX)

set(header_files
    include/rex/assert.hpp
    include/rex/exceptions.hpp
    include/rex/resourceprovider.hpp
)

include_directories("include")
include_directories("thero/include")

#tests
if(BUILD_TESTS)
    include_directories(tests)
    add_executable(tests "tests/main.cpp"
        "tests/helpers/peoplesource.cpp"
        "tests/helpers/toolsource.cpp"
        "tests/resourceprovider.cpp"
        )

    set_property(TARGET tests PROPERTY CXX_STANDARD 14)
    set_property(TARGET tests PROPERTY CXX_STANDARD_REQUIRED ON)

    #target_link_libraries(tests ${project_name})
endif(BUILD_TESTS)

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

if(BUNDLE_THERO)
    install(
        FILES thero/include/thero/optional.hpp thero/include/thero/any.hpp
        DESTINATION ./include/thero/
        )
endif()
