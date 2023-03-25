

function( checkBoost )

    set( 3rdParty_Boost_DIR "${CMAKE_SOURCE_DIR}/3rdParty/Boost/" CACHE PATH "Boost SDK root")

    set( BOOST_ROOT "${3rdParty_Boost_DIR}" )

    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

    set( Boost_DEBUG ON) # enables verbose CMake output
    set( Boost_USE_STATIC_LIBS        OFF)  # only find static libs
    set( Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
    set( Boost_USE_RELEASE_LIBS       ON)  # only find release libs
    set( Boost_USE_MULTITHREADED      ON)
    set( Boost_USE_STATIC_RUNTIME     OFF) # MSVCRT runtime dll or static

    find_package(Boost)

    set( 3rdParty_Boost_INCLUDES "${Boost_INCLUDE_DIR}" CACHE PATH "Boost Include paths to use")
    set( 3rdParty_Boost_LIBS "${Boost_LIBRARIES}" CACHE PATH "Boost Libs to use")

endfunction()

checkBoost()

include_directories( ${3rdParty_Boost_INCLUDES} )
