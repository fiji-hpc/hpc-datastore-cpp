cmake_minimum_required(VERSION 3.18)
project(ds_properties C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -fmax-errors=1")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

# FMT library
find_package(fmt CONFIG REQUIRED)
set(LIBS ${LIBS} fmt::fmt)

# POCO library
find_package(Poco CONFIG REQUIRED Net JSON)
set(LIBS ${LIBS} Poco::Net Poco::JSON)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS} i3dalgo i3dcore)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

