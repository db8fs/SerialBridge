cmake_minimum_required(VERSION 3.26)

project(SerialBridge)

cmake_policy(SET CMP0020 NEW)

include( cmake/Boost.cmake )

include_directories( "${CMAKE_SOURCE_DIR}/src/" )

set( HEADER_FILES  "${CMAKE_SOURCE_DIR}/src/Arguments.h" )
    #"${CMAKE_SOURCE_DIR}/src/SerialPort.h" )

set( SRC_FILES      #"${CMAKE_SOURCE_DIR}/src/SerialPort.cpp"
                    "${CMAKE_SOURCE_DIR}/src/Arguments.cpp"
                    "${CMAKE_SOURCE_DIR}/src/main.cpp" )

add_executable( ${PROJECT_NAME}
                ${HEADER_FILES}
                ${SRC_FILES} )

#target_link_libraries( ${3rdParty_Boost_LIBS} )
