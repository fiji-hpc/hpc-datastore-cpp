cmake_minimum_required(VERSION 3.18)
project(HPCDatastoreClient C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Werror -fmax-errors=1")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

# FMT library
find_package(fmt CONFIG REQUIRED)
set(LIBS ${LIBS} fmt::fmt)

add_executable(main src/main.cpp)
target_link_libraries(main ${LIBS})
set_target_properties(main PROPERTIES CXX_STANDARD 20)

