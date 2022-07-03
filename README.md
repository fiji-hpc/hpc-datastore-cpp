# HPC Datastore C++

This project is up to date with [HPC Datastore server 1.7.0](https://github.com/fiji-hpc/hpc-datastore).

Table of Contents
=================

* [1 Introduction](#1-introduction)
* [2 Instalation](#2-instalation)
  * [2.1 Requirements](#21-requirements)
     * [2.1.1 I3Dlib](#211-i3dlib)
     * [2.1.2 VCPKG](#212-vcpkg)
     * [2.1.3 FMT](#213-fmt)
     * [2.1.4 Poco](#214-poco)
  * [2.2 Inclusion to your project](#22-inclusion-to-your-project)
  * [2.3 Compile settings](#23-compile-settings)
* [3 Documentation](#3-documentation)
* [4 Usage](#4-usage)
  * [4.1 Global (datastore::) scope](#41-global-datastore-scope)
  * [4.2 Connection class](#42-connection-class)
  * [4.3 ImageView class](#43-imageview-class)
* [5 Samples](#5-samples)
  * [5.1 Building sample(s)](#51-building-samples)
* [6 Tests](#6-tests)
  * [6.1 Unit tests](#61-unit-tests)
  * [6.2 Speed tests](#62-speed-tests)
  * [6.3 Building tests](#63-building-tests)

## 1 Introduction

This project is header-only C++ library providing basic communication with [HPC Datastore server](https://github.com/fiji-hpc/hpc-datastore).

It uses [Poco C++ library](https://pocoproject.org) library at the backend to send and recieve HTTP messages and to parse JSON-style data. 
It also uses [FMT](https://fmt.dev/latest/index.html) to make string formatting easier (will be replaced with std::format, 
when [GCC](https://gcc.gnu.org) provides support). 
At last, this library uses [I3D library](https://cbia.fi.muni.cz/software/i3d-library.html) for image representation.

As a result, this projects acts like a wrapper around above mentioned libraries and provides functions to read/write block or images from/to datastore server.   

## 2 Instalation

### 2.1 Requirements
I recommend using [vcpkg](https://github.com/microsoft/vcpkg) to fetch necessary dependencies painlessly 
(all samples are written with **vcpkg** in mind).

#### 2.1.1 I3Dlib
However, as the **i3dlib** is not available from **vcpkg** repositories, you will have to download, build and install library from [official website](https://cbia.fi.muni.cz/software/i3d-library.html). For this particular project, *i3dcore* part of the library is sufficient, though you may want to customize used image compression libraries to enable/disable certain image formats. 
(If you do not want mess with anything, simple *make build* and *sudo make install* should be enough, assuming all neccessary dependencies are installed on your system).

#### 2.1.2 VCPKG
Please follow instructions on installation at the [official website](https://github.com/microsoft/vcpkg).

#### 2.1.3 FMT
**FMT** is available from **vcpkg** repositories and does not have much customizations. Therefore, if you will not use **vcpkg**, just make sure that `fmt::format(...)` is available from `<fmt/core.h>` header.

#### 2.1.4 Poco
**Poco** is quite huge and provides lot of different functionality. As well as **fmt**, **Poco** is obtainable from **vcpkg** repositories. If you will not use **vcpkg**, make sure that parts `Poco::Net` and `Poco::JSON` are available. 

### 2.2 Inclusion to your project
As this library is header-only, the only thing you need is to copy content of `src` directory inside your project and `#include` it.

### 2.3 Compile settings
Of course, you can disable some unwanted features using compile macros.

Available macros:

* **DATASTORE_NDEBUG** (also enabled by **NDEBUG**)

Disables boundary and consistency checks and disables messages.
* **DATASTORE_NLOG**

Disables all messages

* **DATASTORE_NINFO**

Disables info messages

* **DATASTORE_NWARNING**

Disables warning messages

* **DATASTORE_ERROR**

Disables error messages

See samples for an example.

## 3 Documentation

The documentation can be viewed directly in the source code, most importantly in [`hpc_ds_api.hpp`](src/hpc_ds_api.hpp).
There is also doxygen-generated documentation available in [html](https://htmlpreview.github.io/?https://github.com/somik861/hpc-datastore-cpp/blob/main/docs/generated/html/index.html) and [pdf](docs/generated/latex/refman.pdf).

## 4 Usage
The only necessary file to include is [`hpc_ds_api.hpp`](src/hpc_ds_api.hpp).

From that, you will obtain namespace `datastore`, where all functionality is provided. It is recommended you do not use anything outside of root of this namespace. Functions located in `datastore::details` are used for internal functionality.

There are 3 categories of provided api
### 4.1 Global (datastore::) scope
Provides `read_image` and `write_image` functions as well as `get_dataset_properties` to obtain dataset properties from the server.

### 4.2 Connection class
Use this, if you want to connect to different images from one dataset. This class will remember the dataset address and you will not have to write it all over again.

### 4.3 ImageView class
Use this, if you want to connect to one specified image (and use several read/write operations on it). This class will remember the image and you will not have to write it all over again.

## 5 Samples
There exist few samples, that can be used to check, if project is compilable, and to `quick start` your datastore journey.

All samples are located in [`samples` folder](samples) and organized in subdirectories. 

For samples to work properly, you will have to have connection to **HPC datastore server** (It is possible to run it locally on your PC). 
It is also recommended to create testing dataset, so you will not accidentaly overwrite important data. 

Do not forget to write your server *IP*, *PORT*, dataset *uuid* and other properties to [`samples/common.hpp`](samples/common.hpp), where all the sample projects will find it. 

### 5.1 Building sample(s) 
Text bellow assumes you are using **vcpkg**.

You can either build all samples at once, are build them separately.

To build all samples, locate yourself inside [`samples`](samples). To build one sample, locate yourself into appropriate subdirectory. All other instructions are the same.

1. Create build folder (e.g. `mkdir build`)
2. Enter build folder (e.g. `cd build`)
3. Initialize cmake (e.g. `cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg_dir>/scripts/buildsystems/vcpkg.cmake ../`)
4. Build project (e.g. `make`)

After that, all compiled binaries will be located inside `build` folder.

Note, that initialization of **vcpkg** for first time can take some time.

## 6 Tests
Tests samples are divided into two parts: Unit tests and Speed tests.

For all tests to work properly, you will have to have connection to **HPC datastore server** (It is possible to run it locally on your PC).
It is also recommended to create testing dataset, sou you will not accidentaly overwrite important data.

Do not forget to write your *IP*, *PORT*, dataset *uuid* and other properties to [`tests/common.hpp`](tests/common.hpp), where all the tests will find it.

### 6.1 Unit tests
These tests are located in [`tests/units/`](tests/units/) and are created to ensure, that everything is working as expected.

### 6.2 Speed tests
These tests are located in [`tests/speeds/`](tests/speeds/) and are created to measure the speed at which you are able to communicate with server.
There exists 4 different tests:

* Image download
* Image upload
* Block download
* Block upload

The size of the sample to test is equal to the size of image specified in [`tests/common.hpp`](tests/common.hpp) on the server.

### 6.3 Building tests
Text bellow assumes you are using **vcpkg**.

To build the tests, locate yourself into corresponding subfolder ([`units`](tests/units/) or [`speeds`](tests/speeds)).

1. Create build folder (e.g. `mkdir build`)
2. Enter build folder (e.g. `cd build`)
3. Initialize cmake (e.g. `cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg_dir>/scripts/buildsystems/vcpkg.cmake ../`)
4. Build project (e.g. `make`)

After that, all compiled binaries will be located inside `build` folder.

