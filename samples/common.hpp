#pragma once
#include <i3d/vector3d.h>
#include <string>

/* SERVER PROPERTIES  */
const std::string SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 9080;

/* DATASET PROPERTIES */
const std::string DS_UUID = "59c4e076-c4c8-4703-b5ee-fbfdb47d340d";

/* IMAGE PROPERTIES */
constexpr int IMG_CHANNEL = 0;
constexpr int IMG_ANGLE = 0;
constexpr int IMG_TIMEPOINT = 0;
const i3d::Vector3d<int> IMG_RESOLUTION = {0, 0, 0};
const std::string IMG_VERSION = "latest";
