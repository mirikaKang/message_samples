CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(PROJECT_NAME messaging_system)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

OPTION(USE_TYPE_CONTAINER "Contains type container library" ON)

IF(USE_TYPE_CONTAINER)
    ADD_DEFINITIONS(-D__USE_TYPE_CONTAINER__)
ENDIF()

# set the project name
PROJECT(${PROJECT_NAME} VERSION 1.0)

# cpp_libraries
ADD_SUBDIRECTORY(messaging_system/utilities)
IF(USE_TYPE_CONTAINER)
    ADD_SUBDIRECTORY(messaging_system/container)
ENDIF()
ADD_SUBDIRECTORY(messaging_system/threads)
ADD_SUBDIRECTORY(messaging_system/database)
ADD_SUBDIRECTORY(messaging_system/network)

# cpp_samples
ADD_SUBDIRECTORY(logging_sample)
ADD_SUBDIRECTORY(compression_sample)
ADD_SUBDIRECTORY(decompression_sample)
IF(USE_TYPE_CONTAINER)
    ADD_SUBDIRECTORY(container_sample)
ENDIF()
ADD_SUBDIRECTORY(threads_sample)
ADD_SUBDIRECTORY(echo_client)
ADD_SUBDIRECTORY(echo_server)