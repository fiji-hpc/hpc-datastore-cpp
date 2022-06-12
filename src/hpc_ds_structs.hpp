#pragma once
#include <array>
#include <cassert>
#include <fmt/core.h>
#include <i3d/image3d.h>
#include <i3d/vector3d.h>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace datastore {

namespace cnpts {
template <typename T>
concept Scalar = requires(T) {
	requires std::is_scalar_v<T>;
};

template <typename T>
concept Basic = requires(T) {
	requires Scalar<T> || std::is_same_v<T, std::string>;
};

template <typename T>
concept Vector = requires(T) {
	requires std::is_same_v<std::vector<typename T::value_type>, T>;
};

template <typename T>
concept Optional = requires(T) {
	requires std::is_same_v<std::optional<typename T::value_type>, T>;
};

template <typename T>
concept Vector3d = requires(T a) {
	requires std::is_same_v<i3d::Vector3d<decltype(a.x)>, T>;
	requires Basic<decltype(a.x)>;
};

template <typename T>
concept Streamable = requires(T a) {
	{std::cout << a};
};

template <typename T>
concept Map = requires(T) {
	requires std::is_same_v<
	    std::map<typename T::key_type, typename T::mapped_type>, T>;
};
} // namespace cnpts

namespace details {

const inline std::map<std::string, int> type_byte_size{
    {"uint8", 1}, {"uint16", 2}, {"uint32", 4}, {"uint64", 8},  {"int8", 1},
    {"int16", 2}, {"int32", 4},  {"int64", 8},  {"float32", 4}, {"float64", 8}};

constexpr inline std::size_t MAX_URL_LENGTH = 2048;

/** Forward declarations to enable 'recursion' **/
template <cnpts::Streamable T>
std::string to_string(const T&);

template <cnpts::Vector T>
std::string to_string(const T&);

template <cnpts::Map T>
std::string to_string(const T&);

template <cnpts::Optional T>
std::string to_string(const T&);

/** Definitions **/
template <cnpts::Streamable T>
std::string to_string(const T& val) {
	std::stringstream ss;
	ss << val;
	return ss.str();
}

template <cnpts::Vector T>
std::string to_string(const T& vec) {
	std::stringstream ss;
	ss << "(";

	const char* delim = "";
	for (auto& v : vec) {
		ss << delim << to_string(v);
		delim = ", ";
	}

	ss << ")";
	return ss.str();
}

template <cnpts::Map T>
std::string to_string(const T& map) {
	std::stringstream ss;
	ss << "{\n";

	for (const auto& [k, v] : map) {
		ss << to_string(k) << ": " << to_string(v) << '\n';
	}
	ss << "}\n";
	return ss.str();
}

template <cnpts::Optional T>
std::string to_string(const T& val) {
	if (!val)
		return "null";
	return to_string(val.value());
}

} // namespace details

class DatasetProperties {
  public:
	std::string uuid;
	std::string voxel_type;
	i3d::Vector3d<int> dimensions;
	int channels;
	int angles;
	std::optional<std::string> transformations;
	std::string voxel_unit;
	std::optional<i3d::Vector3d<double>> voxel_resolution;
	std::optional<i3d::Vector3d<double>> timepoint_resolution;
	std::optional<i3d::Vector3d<double>> channel_resolution;
	std::optional<i3d::Vector3d<double>> angle_resolution;
	std::string compression;
	std::vector<std::map<std::string, i3d::Vector3d<int>>> resolution_levels;
	std::vector<int> versions;
	std::string label;
	std::optional<std::string> view_registrations;
	std::vector<int> timepoint_ids;

	operator std::string() const {
		std::stringstream ss;
		using details::to_string;

		ss << "UUID: " << uuid << '\n';
		ss << "voxelType: " << voxel_type << '\n';
		ss << "dimensions: " << dimensions << '\n';
		ss << "channels: " << channels << '\n';
		ss << "angles: " << angles << '\n';
		ss << "transformations: " << to_string(transformations) << '\n';
		ss << "voxelUnit: " << voxel_unit << '\n';
		ss << "voxelResolution: " << to_string(voxel_resolution) << '\n';
		ss << "timepointResolution: " << to_string(timepoint_resolution)
		   << '\n';
		ss << "channelResolution: " << to_string(channel_resolution) << '\n';
		ss << "angleResolution: " << to_string(angle_resolution) << '\n';
		ss << "compression: " << compression << '\n';
		ss << "resolutionLevels: " << to_string(resolution_levels) << '\n';
		ss << "versions: " << to_string(versions) << '\n';
		ss << "label: " << label << '\n';
		ss << "viewRegistrations: " << to_string(view_registrations) << '\n';
		ss << "timepointIds: " << to_string(timepoint_ids) << '\n';

		return ss.str();
	}
};
} // namespace datastore
