cmake_minimum_required(VERSION 3.18)
set(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}/../../")

project(read_image CXX)

IF (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
ELSE (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -fmax-errors=1 -g -O2")
set(CMAKE_CXX_COMPILER g++)
ENDIF (WIN32)
set(CMAKE_CXX_STANDARD 20)

# disable info message print
add_compile_options(-DDATASTORE_NINFO)

# FMT library
find_package(fmt CONFIG REQUIRED)
set(LIBS ${LIBS} fmt::fmt)

# POCO library
find_package(Poco CONFIG REQUIRED Net JSON)
set(LIBS ${LIBS} Poco::Net Poco::JSON)

# I3D library
set(LIBS ${LIBS} i3dcore)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})


