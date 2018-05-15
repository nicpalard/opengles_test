cmake_minimum_required(VERSION 3.2)
project(IPOGLES)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/module)

find_package (OpenGLES2 REQUIRED)
set(LIBRARIES GLESv2 EGL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file (GLOB IPOGLES_SOURCES src/*.cpp)
file (GLOB IPOGLES_HEADERS include/*.hpp)

add_executable(ipogles ${IPOGLES_SOURCES} ${IPOGLES_HEADERS})
target_link_libraries(ipogles ${CMAKE_SOURCE_DIR}/lib/libSOIL.a ${LIBRARIES})
