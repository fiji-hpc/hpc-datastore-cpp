#pragma once
#include <array>
#include <cassert>
#include <i3d/image3d.h>
#include <map>
#include <optional>
#include <string>
#include <type_traits>

namespace details {
template <typename T>
concept Scalar = requires(T) {
	std::is_scalar_v<T>;
};

std::string get_dataset_path(const std::string& url, const std::string& uuid);
} // namespace details

namespace datastore {
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
};

DatasetProperties get_dataset_properties(const std::string& url,
                                         const std::string& uuid);

template <details::Scalar T>
i3d::Image3d<T> read_image(const std::string& url,
                           const std::string& uuid,
                           int channel = 0,
                           int timepoint = 0,
                           int angle = 0,
                           Vector3D<int> resolution = {1, 1, 1},
                           const std::string& version = "latest");

template <details::Scalar T>
bool write_image(const i3d::Image3d<T>& img,
                 const std::string& url,
                 const std::string& uuid,
                 int channel = 0,
                 int timepoint = 0,
                 int angle = 0,
                 Vector3D<int> resolution = {1, 1, 1},
                 const std::string version = "latest");

class ImageView {
  public:
	ImageView(std::string url,
	          std::string uuid,
	          int channel,
	          int timepoint,
	          int angle,
	          Vector3D<int> resolution,
	          std::string version);

	template <details::Scalar T>
	i3d::Image3d<T> read_block(Vector3D<int> coords) const;

	template <details::Scalar T>
	bool read_block(Vector3D<int> coords,
	                i3d::Image3d<T>& dest,
	                Vector3D<int> dest_offset = {0, 0, 0}) const;

	template <details::Scalar T>
	std::vector<i3d::Image3d<T>>
	read_blocks(const std::vector<Vector3D<int>>& coords) const;

	template <details::Scalar T>
	bool read_blocks(const std::vector<Vector3D<int>>& coords,
	                 i3d::Image3d<T>& dest,
	                 const std::vector<Vector3D<int>>& offsets) const;

	template <details::Scalar T>
	i3d::Image3d<T> read_image() const;

	template <details::Scalar T>
	bool write_block(const i3d::Image3d<T>& src,
	                 Vector3D<int> coords,
	                 Vector3D<int> src_offset = {0, 0, 0}) const;

	template <details::Scalar T>
	bool write_blocks(const i3d::Image3d<T>& src,
	                  const std::vector<Vector3D<int>>& coords,
	                  const std::vector<Vector3D<int>>& src_offsets) const;

	template <details::Scalar T>
	bool write_image(const i3d::Image3d<T>& img) const;

  private:
	std::string _url;
	std::string _uuid;
	int _channel;
	int _timepoint;
	int _angle;
	Vector3D<int> _resolution;
	std::string _version;
};

class Connection {
  public:
	Connection(std::string url, std::string uuid);

	ImageView get_view(int channel,
	                   int timepoint,
	                   int angle,
	                   Vector3D<int> resolution,
	                   const std::string& version) const;

	template <details::Scalar T>
	i3d::Image3d<T> read_block(Vector3D<int> coords,
	                           int channel,
	                           int timepoint,
	                           int angle,
	                           Vector3D<int> resolution,
	                           const std::string& version) const;

	template <details::Scalar T>
	bool read_block(Vector3D<int> coords,
	                int channel,
	                int timepoint,
	                int angle,
	                Vector3D<int> resolution,
	                const std::string& version,
	                i3d::Image3d<T>& dest,
	                Vector3D<int> dest_offset = {0, 0, 0}) const;

	template <details::Scalar T>
	std::vector<i3d::Image3d<T>>
	read_blocks(const std::vector<Vector3D<int>>& coords,
	            int channel,
	            int timepoint,
	            int angle,
	            Vector3D<int> resolution,
	            const std::string& version) const;

	template <details::Scalar T>
	bool read_blocks(const std::vector<Vector3D<int>>& coords,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 Vector3D<int> resolution,
	                 const std::string& version,
	                 i3d::Image3d<T>& dest,
	                 const std::vector<Vector3D<int>>& dest_offsets) const;

	template <details::Scalar T>
	i3d::Image3d<T> read_image(int channel,
	                           int timepoint,
	                           int angle,
	                           Vector3D<int> resolution,
	                           const std::string& version) const;

	template <details::Scalar T>
	bool write_block(const i3d::Image3d<T>& src,
	                 Vector3D<int> coords,
	                 Vector3D<int> src_offset,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 Vector3D<int> resolution,
	                 const std::string& version) const;

	template <details::Scalar T>
	bool write_blocks(const i3d::Image3d<T>& src,
	                  const std::vector<Vector3D<int>>& coords,
	                  const std::vector<Vector3D<int>>& src_offsets,
	                  int channel,
	                  int timepoint,
	                  int angle,
	                  Vector3D<int> resolution,
	                  const std::string& version) const;

	template <details::Scalar T>
	bool write_image(const i3d::Image3d<T>& src,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 Vector3D<int> resolution,
	                 const std::string& version) const;

  private:
	std::string _url;
	std::string _uuid;
};

} // namespace datastore

/* ================= IMPLEMENTATION FOLLOWS ======================== */
namespace details {
std::string get_dataset_path(const std::string& url, const std::string& uuid) {
	return url + "/datasets/" + uuid;
}
} // namespace details

namespace datastore {
/* ===================================== Global space */
DatasetProperties get_dataset_properties(const std::string& url,
                                         const std::string& uuid) {
	std::string dataset_path = details::get_dataset_path(url, uuid);

	return {};
}

/* ===================================== ImageView */

ImageView::ImageView(std::string url,
                     std::string uuid,
                     int channel,
                     int timepoint,
                     int angle,
                     Vector3D<int> resolution,
                     std::string version)
    : _url(std::move(url)), _uuid(std::move(uuid)), _channel(channel),
      _timepoint(timepoint), _angle(angle), _resolution(resolution),
      _version(std::move(version)) {}

/* ===================================== Connection */

Connection::Connection(std::string url, std::string uuid)
    : _url(std::move(url)), _uuid(std::move(uuid)) {}
} // namespace datastore
