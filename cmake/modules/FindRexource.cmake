set( REXOURCE_FOUND false)

find_path(
    REXOURCE_INCLUDE_DIR
    rex/assert.hpp
    PATH_SUFFIXES
    include
    PATHS
    ${REXOURCE_ROOT}
    ${REXOURCEDIR}
    $ENV{REXOURCEDIR}
    $ENV{REXOURCE_ROOT}
    /usr
    /usr/local
    ~/Library/Frameworks
    /Library/Frameworks
    /sw
    /opt/local
    /opt/csw
    /opt
    "C:/Program Files (x86)"
    "C:/Program Files (x86)/rexource"
)

set(REXOURCE_FOUND)
set(REXOURCE_INCLUDE_DIR)

if(NOT REXOURCE_INCLUDE_DIR)
    set(REXOURCE_FOUND)
    set(REXOURCE_INCLUDE_DIRS)
    if(Rexource_FIND_REQUIRED)
        message(FATAL_ERROR "Rexource not found.")
    elseif(NOT Rexource_FIND_QUIETLY)
        message(STATUS "Rexource not found.")
    endif()
else()
    set(REXOURCE_FOUND true)
    if(NOT Rexource_FIND_QUIETLY)
        message(STATUS "Rexource found: ${REXOURCE_INCLUDE_DIR}")
    endif()
    set(REXOURCE_INCLUDE_DIRS ${REXOURCE_INCLUDE_DIR})
endif()
