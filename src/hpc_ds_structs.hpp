#pragma once
#include <array>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fmt/core.h>
#include <cassert>

namespace datastore
{
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

  private:
	std::array<T, 3> _values;
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

		// TODO finish
		ss << "UUID: " << uuid << '\n';
		ss << "VoxelType: " << voxel_type << '\n';

		return ss.str();
	}
};
}