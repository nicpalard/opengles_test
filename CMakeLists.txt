cmake_minimum_required (VERSION 3.2)
project (IPOGLES)

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/module)

find_package (OpenGLES2 REQUIRED)
if (NOT OpenGLES2_FOUND)
	message(FATAL_ERROR, "OpenGLES 2.0 could not be found")
endif()
set (LIBRARIES GLESv2 EGL)

find_package( OpenCV REQUIRED core opencv_imgcodecs opencv_videoio)
if (NOT OPENCV_FOUND)
        message(FATAL_ERROR, "OpenCV could not be found.")
endif()

find_package(redisimagehelper)
if (NOT REDISIMAGEHELPER_FOUND)
	message("redisimagehelper could not be found.")
endif()

find_package(hiredis)
if (NOT HIREDIS_FOUND)
	message("redisimagehelper could not be found.")
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

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

if (REDISIMAGEHELPER_FOUND AND HIREDIS_FOUND)
	include_directories (${REDISIMAGEHELPER_INCLUDE_DIR})
	add_executable (ip_redis ${REDIS_SOURCES} ${HEADERS})
	target_link_libraries (ip_redis ${LIBRARIES} ${OpenCV_LIBS} ${HIREDIS_LIBS} ${REDISIMAGEHELPER_LIBS})
endif()

add_custom_target (shaders ${SHADERS})
