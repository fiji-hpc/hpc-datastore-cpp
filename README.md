# HPC Datastore C++

This project is up to date with [HPC Datastore server 1.7.0](https://github.com/fiji-hpc/hpc-datastore).

## Introduction

This project is header-only C++ library providing basic communication with [HPC Datastore server](https://github.com/fiji-hpc/hpc-datastore).

It uses [Poco C++ library](https://pocoproject.org) library at the backend to send and recieve HTTP messages and to parse JSON-style data. 
It also uses [FMT](https://fmt.dev/latest/index.html) to make string formatting easier (will be replaced with std::format, 
when [GCC](https://gcc.gnu.org) provides support). 
At last, this library uses [I3D library](https://cbia.fi.muni.cz/software/i3d-library.html) for image representation.

As a result, this projects acts like a wrapper around above mentioned libraries and provides functions to read/write block or images from/to datastore server.   

## Instalation

### Requirements
I recommend using [vcpkg](https://github.com/microsoft/vcpkg) to fetch necessary dependencies painlessly 
(all samples are written with **vcpkg** in mind).

#### I3Dlib
However, as the **i3dlib** is not available from **vcpkg** repositories, you will have to download, build and install library from [official website](https://cbia.fi.muni.cz/software/i3d-library.html). For this particular project, *i3dcore* part of the library is sufficient, though you may want to customize used image compression libraries to enable/disable certain image formats. 
(If you do not want mess with anything, simple *make build* and *sudo make install* should be enough, assuming all neccessary dependencies are installed on your system).

#### VCPKG
Please follow instructions on installation at the [official website](https://github.com/microsoft/vcpkg).

#### FMT
**FMT** is available from **vcpkg** repositories and does not have much customizations. Therefore, if you will not use **vcpkg**, just make sure that `fmt::format(...)` is available from `<fmt/core.h>` header.

#### Poco
**Poco** is quite huge and provides lot of different functionality. As well as **fmt**, **Poco** is obtainable from **vcpkg** repositories. If you will not use **vcpkg**, make sure that parts `Poco::Net` and `Poco::JSON` are available. 

### Inclusion to your project
As this library is header-only, the only thing you need is to copy content of `src` directory inside your project and `#include` it.

## Documentation

The documentation can be viewed directly in the source code, most importantly in [`hpc_ds_api.hpp`](src/hpc_ds_api.hpp).
There is also doxygen-generated documentation available in [html](docs/generated/html/index.html) and [pdf](docs/generated/latex/refman.pdf).

## Usage
The only necessary file to include is [`hpc_ds_api.hpp`](src/hpc_ds_api.hpp).

From that, you will obtain namespace `datastore`, where all functionality is provided. It is recommended you do not use anything outside of root of this namespace. Functions located in `datastore::details` are used for internal functionality.

There are 3 categories of provided api
### Global (datastore::) scope
Provides `read_image` and `write_image` functions as well as `get_dataset_properties` to obtain dataset properties from the server.

### Connection class
Use this, if you want to connect to different images from one dataset. This class will remember the dataset address and you will not have to write it all over again.

### ImageView class
Use this, if you want to connect to one specified image (and use several read/write operations on it). This class will remember the image and you will not have to write it all over again.

## Samples
There exist few samples, that can be used to check, if project is compilable, and to `quick start` your datastore journey.

All samples are located in [`samples` folder](samples) and organized in subdirectories. 

For samples to work properly, you will have to have connection to **HPC datastore server** (It is possible to run it locally on your PC). 
It is also recommended to create testing dataset, so you will not accidentaly overwrite important data. 

Do not forget to write your server *IP*, *PORT* and dataset *uuid* to [`samples/common.hpp`](samples/common.hpp), where all the sample projects will find it. 

### Building sample(s) 
Text bellow assumes you are using **vcpkg**.

You can either build all samples at once, are build them separately.

To build all samples, locate yourself inside [`samples`](samples). To build one sample, locate yourself into appropriate subdirectory. All other instructions are the same.

1. Create build folder (e.g. `mkdir build`)
2. Enter build folder (e.g. `cd build`)
3. Initialize cmake (e.g. `cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg_dir>/scripts/builsystems/vcpkg.cmake` ../)
4. Build project (e.g. `make`)

After that, all compiled binaries will be located inside `build` folder.

Note, that initialization of **vcpkg** for first time can take some time.

## Tests

