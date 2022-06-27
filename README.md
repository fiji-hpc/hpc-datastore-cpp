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

#### FMT, POCO


## Documentation

## Usage

## Samples

## Tests
