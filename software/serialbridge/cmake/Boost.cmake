

function( checkBoost )

    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

    set( Boost_DEBUG ON) # enables verbose CMake output
    set( Boost_USE_STATIC_LIBS        OFF)

if(WIN32)
    set( Boost_USE_STATIC_LIBS        ON)  # use static libs for win32
endif(WIN32)

    set( Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
    set( Boost_USE_RELEASE_LIBS       ON)  # only find release libs
    set( Boost_USE_MULTITHREADED      ON)
    set( Boost_USE_STATIC_RUNTIME     OFF) # MSVCRT runtime dll or static

    find_package(Boost 1.60.0 COMPONENTS program_options system thread)

endfunction()

checkBoost()

include_directories( ${Boost_INCLUDE_DIRS} )
