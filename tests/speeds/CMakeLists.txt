cmake_minimum_required(VERSION 3.18)
set(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}/../../")

project(HPC_Datastore_speed_tests CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -fmax-errors=1 -O2 -g")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)

# disable debugging
add_compile_options(-DNDEBUG)


# FMT library
find_package(fmt CONFIG REQUIRED)
set(LIBS ${LIBS} fmt::fmt)

# POCO library
find_package(Poco CONFIG REQUIRED Net JSON)
set(LIBS ${LIBS} Poco::Net Poco::JSON)

# I3D library
set(LIBS ${LIBS} i3dcore)

foreach(PROJ read_image) #read_blocks read_image write_blocks write_image)
    add_executable(${PROJ} "${PROJ}.cpp")
    target_link_libraries(${PROJ} PRIVATE ${LIBS})
endforeach()



