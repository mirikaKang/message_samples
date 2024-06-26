CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(PROJECT_NAME messaging_system)

set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    OPTION(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
ELSE()
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "../../../lib")
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../bin")
    OPTION(BUILD_SHARED_LIBS "Build using shared libraries" ON)
ENDIF()

OPTION(USE_UNIT_TEST "Use unit test" ON)

# set the project name
PROJECT(${PROJECT_NAME} VERSION 1.0)

# cpp_libraries
ADD_SUBDIRECTORY(messaging_system/utilities)
ADD_SUBDIRECTORY(messaging_system/container)
ADD_SUBDIRECTORY(messaging_system/threads)
ADD_SUBDIRECTORY(messaging_system/database)
ADD_SUBDIRECTORY(messaging_system/network)
IF(USE_UNIT_TEST)
    ADD_SUBDIRECTORY(messaging_system/unittest)
ENDIF()

# cpp_samples
ADD_SUBDIRECTORY(logging_sample)
ADD_SUBDIRECTORY(container_sample)
ADD_SUBDIRECTORY(threads_sample)
ADD_SUBDIRECTORY(echo_client)
ADD_SUBDIRECTORY(echo_server)