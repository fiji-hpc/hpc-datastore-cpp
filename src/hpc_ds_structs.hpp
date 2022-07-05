#pragma once
#include <array>
#include <cassert>
#include <fmt/core.h>
#include <i3d/image3d.h>
#include <i3d/transform.h>
#include <i3d/vector3d.h>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

template <typename T, typename U>
bool lt(i3d::Vector3d<T> lhs, i3d::Vector3d<U> rhs) {
	return lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z;
}

template <typename T, typename U>
bool eq(i3d::Vector3d<T> lhs, i3d::Vector3d<U> rhs) {
	for (int i = 0; i < 3; ++i)
		if (static_cast<long long>(lhs[i]) != static_cast<long long>(rhs[i]))
			return false;
	return true;
}

namespace ds {

using i3d::SamplingMode;

/* dataset 'voxel_type' to 'byte_size' map*/
const inline std::map<std::string, int> type_byte_size{
    {"uint8", 1}, {"uint16", 2}, {"uint32", 4}, {"uint64", 8},  {"int8", 1},
    {"int16", 2}, {"int32", 4},  {"int64", 8},  {"float32", 4}, {"float64", 8}};

/* Maximal legal URL length */
constexpr inline std::size_t MAX_URL_LENGTH = 2048;

/**
 * @brief Class representing resolution unit (in DatasetProperties)
 *
 */
class ResolutionUnit {
  public:
	double value = 0.0;
	std::string unit = "";

	friend std::ostream& operator<<(std::ostream& stream,
	                                const ResolutionUnit& res) {
		stream << fmt::format("{} {}", res.value, res.unit);
		return stream;
	}
};

/* Concepts definitions to make templates more readable */
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

template <typename T>
concept ResolutionUnit = requires(T) {
	requires std::is_same_v<T, ds::ResolutionUnit>;
};
} // namespace cnpts

namespace details {

/** Unified way to convert structures into string **/
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
	ss << "}";
	return ss.str();
}

template <cnpts::Optional T>
std::string to_string(const T& val) {
	if (!val)
		return "null";
	return to_string(val.value());
}

} // namespace details

/**
 * @brief Class representing dataset properties
 *
 */
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
	std::optional<ResolutionUnit> timepoint_resolution;
	std::optional<ResolutionUnit> channel_resolution;
	std::optional<ResolutionUnit> angle_resolution;
	std::string compression;
	std::vector<std::map<std::string, i3d::Vector3d<int>>> resolution_levels;
	std::vector<int> versions;
	std::string label;
	std::optional<std::string> view_registrations;
	std::vector<int> timepoint_ids;

	i3d::Vector3d<int>
	get_block_dimensions(i3d::Vector3d<int> resolution) const {
		for (const auto& map : resolution_levels)
			if (map.at("resolutions") == resolution)
				return map.at("blockDimensions");

		std::string msg = fmt::format("Resolution {} not found in properties",
		                              details::to_string(resolution));
		throw std::out_of_range(msg.c_str());
	}

	i3d::Vector3d<int> get_block_size(i3d::Vector3d<int> coord,
	                                  i3d::Vector3d<int> resolution) const {
		i3d::Vector3d<int> block_dim = get_block_dimensions(resolution);
		i3d::Vector3d<int> start = (coord * block_dim);
		i3d::Vector3d<int> end = (coord + 1) * block_dim;

		i3d::Vector3d<int> out;
		for (int i = 0; i < 3; ++i) {
			out[i] = std::max(0, std::min(dimensions[i], end[i]) -
			                         std::max(start[i], 0));
		}
		return out;
	}

	i3d::Vector3d<int> get_block_count(i3d::Vector3d<int> resolution) const {
		i3d::Vector3d<int> block_dim = get_block_dimensions(resolution);

		return (dimensions + block_dim - 1) / block_dim;
	}

	i3d::Vector3d<int> get_img_dimensions(i3d::Vector3d<int> resolution) {
		return dimensions / resolution;
	}

	friend std::ostream& operator<<(std::ostream& stream,
	                                const DatasetProperties& ds) {
		using details::to_string;

		stream << "UUID: " << ds.uuid << '\n';
		stream << "voxelType: " << ds.voxel_type << '\n';
		stream << "dimensions: " << ds.dimensions << '\n';
		stream << "channels: " << ds.channels << '\n';
		stream << "angles: " << ds.angles << '\n';
		stream << "transformations: " << to_string(ds.transformations) << '\n';
		stream << "voxelUnit: " << ds.voxel_unit << '\n';
		stream << "voxelResolution: " << to_string(ds.voxel_resolution) << '\n';
		stream << "timepointResolution: " << to_string(ds.timepoint_resolution)
		       << '\n';
		stream << "channelResolution: " << to_string(ds.channel_resolution)
		       << '\n';
		stream << "angleResolution: " << to_string(ds.angle_resolution) << '\n';
		stream << "comprestreamion: " << ds.compression << '\n';
		stream << "resolutionLevels: " << to_string(ds.resolution_levels)
		       << '\n';
		stream << "versions: " << to_string(ds.versions) << '\n';
		stream << "label: " << ds.label << '\n';
		stream << "viewRegistrations: " << to_string(ds.view_registrations)
		       << '\n';
		stream << "timepointIds: " << to_string(ds.timepoint_ids) << '\n';

		return stream;
	}
};
} // namespace ds
