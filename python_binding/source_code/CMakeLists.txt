cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

project(HPC_Datastore_pybind CXX)

# disable info message print
add_compile_options(-DDATASTORE_NINFO)

# FMT library
find_package(fmt CONFIG REQUIRED)
set(LIBS ${LIBS} fmt::fmt)

# POCO library
find_package(Poco CONFIG REQUIRED Net JSON)
set(LIBS ${LIBS} Poco::Net Poco::JSON)

# FFTW3 library
find_package(FFTW3 CONFIG REQUIRED)
set(LIBS ${LIBS} FFTW3::fftw3)

find_package(FFTW3f CONFIG REQUIRED)
set(LIBS ${LIBS} FFTW3::fftw3f)

find_package(FFTW3l CONFIG REQUIRED)
set(LIBS ${LIBS} FFTW3::fftw3l)

# Clapack library
find_package(clapack CONFIG REQUIRED)
set(LIBS ${LIBS} lapack)

# F2C library
if(UNIX)
	if(NOT APPLE)
		set(LIBS ${LIBS} f2c)
	endif(NOT APPLE)
endif(UNIX)

# BLAS library
find_package(OpenBLAS CONFIG REQUIRED)
set(LIBS ${LIBS} OpenBLAS::OpenBLAS)

# TIFF library
find_package(TIFF REQUIRED)
set(LIBS ${LIBS} ${TIFF_LIBRARIES})

# JPEG library
find_package(JPEG REQUIRED)
set(LIBS ${LIBS} ${JPEG_LIBRARIES})

# PNG library
find_package(libpng CONFIG REQUIRED)

if(WIN32)
	set(LIBS ${LIBS} png)
else(WIN32)
	set(LIBS ${LIBS} png_static)
endif(WIN32)

# ICS library
find_package(libics CONFIG REQUIRED)
set(LIBS ${LIBS} libics)

# ZLIB library
find_package(ZLIB REQUIRED)
set(LIBS ${LIBS} ZLIB::ZLIB)

# HDF5 library
find_package(hdf5 CONFIG REQUIRED)

if(WIN32)
	set(LIBS ${LIBS} hdf5::hdf5-shared hdf5::hdf5_hl-shared)
else(WIN32)
	set(LIBS ${LIBS} hdf5::hdf5-static hdf5::hdf5_hl-static)
endif(WIN32)

# RXSpencer library
find_package(RXSpencer CONFIG REQUIRED)
set(LIBS ${LIBS} rxspencer)

# I3D library
set(STATIC_DIR "${CMAKE_SOURCE_DIR}/../../static_i3dlib")
set(LIBS "${STATIC_DIR}/libs/libi3dcore.a" "${STATIC_DIR}/libs/libi3dalgo.a" ${LIBS})
set(INCL_DIRS "${STATIC_DIR}/includes")

# Pybind11 library
add_subdirectory("${CMAKE_SOURCE_DIR}/pybind11")
set(INCL_DIRS "${CMAKE_SOURCE_DIR}/pybind11/include")

set(MODULE_NAME "hpc_datastore")
pybind11_add_module(${MODULE_NAME} src/main.cpp)
target_link_libraries(${MODULE_NAME} PUBLIC ${LIBS})
target_include_directories(${MODULE_NAME} SYSTEM PUBLIC ${INCL_DIRS})

# add compile options
set_target_properties(${MODULE_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wconversion -fmax-errors=1 -g -O2 -fPIC")
