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
void shuffle(T& cont) {
	std::mt19937_64 gen{std::random_device()()};

	std::ranges::shuffle(cont, gen);
}

void test_start(const std::string& test_name) {
	std::string border(10, '=');
	std::cout << border + " " + test_name + " " + border << '\n';
}
void test_ok() {
	std::string border(10, '-');
	std::cout << border + " [ SUCCESS ] " + border << '\n';
}
void test_nok() {
	std::string border(10, '-');
	std::cout << border + " [ FAILED ] " + border << '\n';
}

void phase_ok() { std::cout << "[OK]" << std::endl; }
void phase_nok() { std::cout << "[NOK]" << std::endl; }
void phase_start(const std::string& s) {
	std::cout << "Phase: " << s << " ... " << std::flush;
}

template <typename T>
bool equal_subimage(const i3d::Image3d<T>& haystack,
                    const i3d::Image3d<T>& needle,
                    i3d::Vector3d<int> offset) {
	for (std::size_t x = 0; x < needle.GetSizeX(); ++x)
		for (std::size_t y = 0; y < needle.GetSizeY(); ++y)
			for (std::size_t z = 0; z < needle.GetSizeZ(); ++z) {
				i3d::Vector3d<std::size_t> coord{x, y, z};
				if (haystack.GetVoxel(coord + offset) != needle.GetVoxel(coord))
					return false;
			}

	return true;
}

template <typename T>
void copy_to_subimage(i3d::Image3d<T>& haystack,
                      const i3d::Image3d<T> needle,
                      i3d::Vector3d<int> offset) {
	for (std::size_t x = 0; x < needle.GetSizeX(); ++x)
		for (std::size_t y = 0; y < needle.GetSizeY(); ++y)
			for (std::size_t z = 0; z < needle.GetSizeZ(); ++z) {
				i3d::Vector3d<std::size_t> coord{x, y, z};
				haystack.SetVoxel(coord + offset, needle.GetVoxel(coord));
			}
}

template <typename T>
i3d::Image3d<T> get_subimage(const i3d::Image3d<T>& src,
                             i3d::Vector3d<int> start,
                             i3d::Vector3d<int> size) {
	i3d::Image3d<T> out;
	out.MakeRoom(size);

	for (std::size_t x = 0; x < out.GetSizeX(); ++x)
		for (std::size_t y = 0; y < out.GetSizeY(); ++y)
			for (std::size_t z = 0; z < out.GetSizeZ(); ++z) {
				i3d::Vector3d coord = {x, y, z};
				out.SetVoxel(coord, src.GetVoxel(coord + start));
			}

	return out;
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
