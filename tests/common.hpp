#pragma once
#include <i3d/image3d.h>
#include <i3d/vector3d.h>
#include <random>
#include <string>
#include <utility>

/** DO NOT FORGET TO CHANGE THESE TO MATCH YOUR SERVER **/

/* SERVER PROPERTIES  */
const std::string SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 9080;

/* DATASET PROPERTIES */
const std::string DS_UUID = "59c4e076-c4c8-4703-b5ee-fbfdb47d340d";

/* IMAGE PROPERTIES */
constexpr int IMG_CHANNEL = 0;
constexpr int IMG_ANGLE = 0;
constexpr int IMG_TIMEPOINT = 0;
const i3d::Vector3d<int> IMG_RESOLUTION = {1, 1, 1};
const std::string IMG_VERSION = "latest";

/** Some functionality shared between samples **/

/**
 * @brief Fill image with random values
 *
 * @tparam T Scalar image element type
 * @param img image
 */
template <typename T>
void fill_random(i3d::Image3d<T>& img) {
	std::mt19937_64 gen{std::random_device()()};
	std::uniform_int_distribution<long long> dist;

	for (auto& voxel : img)
		voxel = T(dist(gen));
}

template <typename T>
void shuffle(T& cont)
{
	std::mt19937_64 gen{std::random_device()()};
	
	std::ranges::shuffle(cont, gen);
}

template <typename T>
bool operator==(const i3d::Image3d<T>& rhs, const i3d::Image3d<T>& lhs) {
	if (rhs.GetSize() != lhs.GetSize())
		return false;

	for (std::size_t i = 0; i < rhs.GetImageSize(); ++i)
		if (rhs.GetVoxel(i) != lhs.GetVoxel(i))
			return false;

	return true;
}

#define SELECT_TYPE(type, func)                                                \
	if (type == "uint8")                                                       \
		func<uint8_t>();                                                       \
	else if (type == "uint16")                                                 \
		func<uint16_t>();                                                      \
	else if (type == "uint64")                                                 \
		func<uint64_t>();                                                      \
	else if (type == "int32")                                                  \
		func<int32_t>();                                                       \
	else if (type == "float32")                                                \
		func<float>();                                                         \
	else if (type == "float64")                                                \
		func<double>();                                                        \
	else                                                                       \
		std::cout << "Image type is not supported !! \n";
