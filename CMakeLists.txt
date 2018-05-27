cmake_minimum_required (VERSION 3.2)
project (IPOGLES)

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/module)

find_package (OpenGLES2 REQUIRED)
set (LIBRARIES GLESv2 EGL)

find_package( OpenCV REQUIRED core opencv_imgcodecs opencv_videoio)
if (NOT OPENCV_FOUND)
        message(FATAL_ERROR, "OpenCV not found.")
endif()

find_package(rediscamclient REQUIRED)
if (NOT REDISCAMCLIENT_FOUND)
        message(FATAL_ERROR, "rediscamclient not found.")
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories (${REDISCAMCLIENT_INCLUDE_DIR})

file (GLOB SOURCES src/*.cpp)
file (GLOB HEADERS include/*.hpp)
file (GLOB SHADERS shader/*)

set (IPO_SOURCES
        src/ipogles.cpp
        src/quad.cpp
)

set (BENCH_SOURCES
        src/bench.cpp
        src/quad.cpp
)

set (REDIS_SOURCES
        src/redis.cpp
        src/RedisCameraServer.cpp
        src/quad.cpp
)

add_executable (ipogles ${IPO_SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries (ipogles ${LIBRARIES} ${OpenCV_LIBS})

add_executable (ip_bench ${BENCH_SOURCES} ${HEADERS})
target_link_libraries (ip_bench ${LIBRARIES})

add_executable (ip_redis ${REDIS_SOURCES} ${HEADERS})
target_link_libraries (ip_redis ${LIBRARIES} ${OpenCV_LIBS} hiredis ${REDISCAMCLIENT_LIBS})

add_custom_target (shaders ${SHADERS})
