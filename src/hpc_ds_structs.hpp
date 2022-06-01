#pragma once
#include <array>
#include <cassert>
#include <fmt/core.h>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace datastore {

namespace details {
template <typename key_t, typename value_t>
std::ostream& operator<<(std::ostream& stream,
                         const std::map<key_t, value_t>& map) {
	stream << "{\n";

	for (const auto& [k, v] : map) {
		stream << k << ": " << v << '\n';
	}
	stream << "}\n";
	return stream;
}

template <typename T>
std::string vec_to_string(const std::vector<T>& vec) {
	std::stringstream ss;
	ss << "[";
	const char* delim = "";
	for (auto& v : vec) {
		ss << delim << v;
		delim = ", ";
	}

	ss << "]";
	return ss.str();
}
} // namespace details

template <typename T>
concept Scalar = requires(T) {
	std::is_scalar_v<T>;
};

template <typename T>
class Vector3D {
  public:
	Vector3D() = default;
	Vector3D(T xyz) : _values({xyz, xyz, xyz}) {}
	Vector3D(T x, T y, T z) : _values({x, y, z}) {}

	T& x() { return _values[0]; }
	T& y() { return _values[1]; }
	T& z() { return _values[2]; }

	T x() const { return _values[0]; }
	T y() const { return _values[1]; }
	T z() const { return _values[2]; }

	T& operator[](std::size_t idx) {
		assert(0 <= idx && idx < 3);
		return _values[idx];
	}
	T operator[](std::size_t idx) const {
		assert(0 <= idx && idx < 3);
		return _values[idx];
	}

	operator std::string() const {
		return fmt::format("[{}, {}, {}]", x(), y(), z());
	}

	friend std::ostream& operator<<(std::ostream& stream, Vector3D vec) {
		stream << std::string(vec);
		return stream;
	}

  private:
	std::array<T, 3> _values{};
};

class DatasetProperties {
  public:
	std::string uuid;
	std::string voxel_type;
	Vector3D<int> dimensions;
	int channels;
	int angles;
	std::optional<std::string> transformations;
	std::string voxel_unit;
	Vector3D<double> voxel_resolution;
	std::optional<Vector3D<double>> timepoint_resolution;
	std::optional<Vector3D<double>> channel_resolution;
	std::optional<Vector3D<double>> angle_resolution;
	std::string compression;
	std::vector<std::map<std::string, Vector3D<int>>> resolution_levels;
	std::vector<int> versions;
	std::string label;
	std::optional<std::string> view_registrations;
	std::vector<int> timepoint_ids;

	operator std::string() const {
		std::stringstream ss;

		ss << "UUID: " << uuid << '\n';
		ss << "voxelType: " << voxel_type << '\n';
		ss << "dimensions: " << std::string(dimensions) << '\n';
		ss << "channels: " << channels << '\n';
		ss << "angles: " << angles << '\n';
		ss << "transformations: " << transformations.value_or("null") << '\n';
		ss << "voxelUnit: " << voxel_unit << '\n';
		ss << "voxelResolution: " << std::string(voxel_resolution) << '\n';
		ss << "timepointResolution: "
		   << (timepoint_resolution ? std::string(timepoint_resolution.value())
		                            : "null")
		   << '\n';

		ss << "channelResolution: "
		   << (channel_resolution ? std::string(channel_resolution.value())
		                          : "null")
		   << '\n';

		ss << "angleResolution: "
		   << (angle_resolution ? std::string(angle_resolution.value())
		                        : "null")
		   << '\n';

		ss << "compression: " << compression << '\n';
		ss << "resolutionLevels: " << details::vec_to_string(resolution_levels)
		   << '\n';
		ss << "versions: " << details::vec_to_string(versions) << '\n';
		ss << "label: " << label << '\n';
		ss << "viewRegistrations: " << view_registrations.value_or("null")
		   << '\n';
		ss << "timepointIds: " << details::vec_to_string(timepoint_ids) << '\n';

		return ss.str();
	}
};
} // namespace datastore
