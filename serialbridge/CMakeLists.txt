cmake_minimum_required(VERSION 3.0)

project(SerialBridge)

cmake_policy(SET CMP0020 NEW)

#set( Boost_ROOT "C:/Projekte/3rdParty/boost_1_79_0/" )

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

target_link_libraries( SerialBridge Boost::system Boost::program_options )
