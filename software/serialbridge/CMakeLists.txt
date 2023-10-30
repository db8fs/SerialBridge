cmake_minimum_required(VERSION 3.0)

project(SerialBridge VERSION 0.9.0)

cmake_policy(SET CMP0020 NEW)

#set( Boost_ROOT "C:/Projekte/3rdParty/boost_1_79_0/" )

include( cmake/Boost.cmake )

include_directories( "${CMAKE_SOURCE_DIR}/src/" )

set( HEADER_FILES  "${CMAKE_SOURCE_DIR}/src/Arguments.h" 
                   "${CMAKE_SOURCE_DIR}/src/INetworkHandler.h"
                   "${CMAKE_SOURCE_DIR}/src/NetworkConnection.h"
                   "${CMAKE_SOURCE_DIR}/src/SerialBridge.h"
                   "${CMAKE_SOURCE_DIR}/src/SerialPort.h"
                   "${CMAKE_SOURCE_DIR}/src/System.h"
                   "${CMAKE_SOURCE_DIR}/src/NetworkServer.h" )

set( SRC_FILES      "${CMAKE_SOURCE_DIR}/src/Arguments.cpp"
                    "${CMAKE_SOURCE_DIR}/src/SerialBridge.cpp"
                    "${CMAKE_SOURCE_DIR}/src/SerialPort.cpp"
                    "${CMAKE_SOURCE_DIR}/src/System.cpp"
                    "${CMAKE_SOURCE_DIR}/src/NetworkServer.cpp"
                    "${CMAKE_SOURCE_DIR}/src/main.cpp" )

add_executable( ${PROJECT_NAME}
                ${HEADER_FILES}
                ${SRC_FILES} )

target_link_libraries( SerialBridge Boost::serialization Boost::program_options Boost::thread Boost::filesystem)

install(TARGETS SerialBridge RUNTIME DESTINATION bin)


if(UNIX)
  execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("Architecture: ${ARCHITECTURE}")
  set(CPACK_GENERATOR "DEB")
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/db8fs/SerialBridge.git")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "db8fs")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev, libboost-thread-dev, libboost-program-options-dev, libboost-serialization-dev, libboost-system-dev, libboost-filesystem-dev")
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
  include(CPack)
endif(UNIX)
