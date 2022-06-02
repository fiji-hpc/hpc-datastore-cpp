#pragma once
#include "hpc_ds_details.hpp"
#include "hpc_ds_structs.hpp"
#include <fmt/core.h>
#include <i3d/image3d.h>
#include <string>
#include <type_traits>
#include <vector>

namespace datastore {
inline DatasetProperties get_dataset_properties(const std::string& ip,
                                                int port,
                                                const std::string& uuid);

// TODO finish implementation
// TODO docs
template <Scalar T>
i3d::Image3d<T> read_image(const std::string& ip,
                           int port,
                           const std::string& uuid,
                           int channel = 0,
                           int timepoint = 0,
                           int angle = 0,
                           Vector3D<int> resolution = {1, 1, 1},
                           const std::string& version = "latest");

// TODO finish implementation
// TODO docs
template <Scalar T>
bool write_image(const i3d::Image3d<T>& img,
                 const std::string& ip,
                 int port,
                 const std::string& uuid,
                 int channel = 0,
                 int timepoint = 0,
                 int angle = 0,
                 Vector3D<int> resolution = {1, 1, 1},
                 const std::string version = "latest");

class ImageView {
  public:
	ImageView(std::string ip,
	          int port,
	          std::string uuid,
	          int channel,
	          int timepoint,
	          int angle,
	          Vector3D<int> resolution,
	          std::string version);

	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	i3d::Image3d<T> read_block(Vector3D<int> coords) const;

	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool read_block(Vector3D<int> coords,
	                i3d::Image3d<T>& dest,
	                Vector3D<int> dest_offset = {0, 0, 0}) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	std::vector<i3d::Image3d<T>>
	read_blocks(const std::vector<Vector3D<int>>& coords) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool read_blocks(const std::vector<Vector3D<int>>& coords,
	                 i3d::Image3d<T>& dest,
	                 const std::vector<Vector3D<int>>& offsets) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	i3d::Image3d<T> read_image() const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool write_block(const i3d::Image3d<T>& src,
	                 Vector3D<int> coords,
	                 Vector3D<int> src_offset = {0, 0, 0}) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool write_blocks(const i3d::Image3d<T>& src,
	                  const std::vector<Vector3D<int>>& coords,
	                  const std::vector<Vector3D<int>>& src_offsets) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool write_image(const i3d::Image3d<T>& img) const;

  private:
	std::string _ip;
	int _port;
	std::string _uuid;
	int _channel;
	int _timepoint;
	int _angle;
	Vector3D<int> _resolution;
	std::string _version;
};

class Connection {
  public:
	// TODO finish implementation
	// TODO docs
	Connection(std::string ip, int port, std::string uuid);
	// TODO finish implementation
	// TODO docs
	ImageView get_view(int channel,
	                   int timepoint,
	                   int angle,
	                   Vector3D<int> resolution,
	                   const std::string& version) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	i3d::Image3d<T> read_block(Vector3D<int> coords,
	                           int channel,
	                           int timepoint,
	                           int angle,
	                           Vector3D<int> resolution,
	                           const std::string& version) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool read_block(Vector3D<int> coords,
	                int channel,
	                int timepoint,
	                int angle,
	                Vector3D<int> resolution,
	                const std::string& version,
	                i3d::Image3d<T>& dest,
	                Vector3D<int> dest_offset = {0, 0, 0}) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	std::vector<i3d::Image3d<T>>
	read_blocks(const std::vector<Vector3D<int>>& coords,
	            int channel,
	            int timepoint,
	            int angle,
	            Vector3D<int> resolution,
	            const std::string& version) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool read_blocks(const std::vector<Vector3D<int>>& coords,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 Vector3D<int> resolution,
	                 const std::string& version,
	                 i3d::Image3d<T>& dest,
	                 const std::vector<Vector3D<int>>& dest_offsets) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	i3d::Image3d<T> read_image(int channel,
	                           int timepoint,
	                           int angle,
	                           Vector3D<int> resolution,
	                           const std::string& version) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool write_block(const i3d::Image3d<T>& src,
	                 Vector3D<int> coords,
	                 Vector3D<int> src_offset,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 Vector3D<int> resolution,
	                 const std::string& version) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool write_blocks(const i3d::Image3d<T>& src,
	                  const std::vector<Vector3D<int>>& coords,
	                  const std::vector<Vector3D<int>>& src_offsets,
	                  int channel,
	                  int timepoint,
	                  int angle,
	                  Vector3D<int> resolution,
	                  const std::string& version) const;
	// TODO finish implementation
	// TODO docs
	template <Scalar T>
	bool write_image(const i3d::Image3d<T>& src,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 Vector3D<int> resolution,
	                 const std::string& version) const;

  private:
	std::string _ip;
	int _port;
	std::string _uuid;
};

} // namespace datastore

/* ================= IMPLEMENTATION FOLLOWS ======================== */

namespace datastore {
/* ===================================== Global space */
/* inline */ DatasetProperties get_dataset_properties(const std::string& ip,
                                                      int port,
                                                      const std::string& uuid) {
	std::string json = details::get_dataset_json_str(ip, port, uuid);
	return details::get_properties_from_json_str(json);
}

/* ===================================== ImageView */

ImageView::ImageView(std::string ip,
                     int port,
                     std::string uuid,
                     int channel,
                     int timepoint,
                     int angle,
                     Vector3D<int> resolution,
                     std::string version)
    : _ip(std::move(ip)), _port(port), _uuid(std::move(uuid)),
      _channel(channel), _timepoint(timepoint), _angle(angle),
      _resolution(resolution), _version(std::move(version)) {}

template <Scalar T>
bool ImageView::read_blocks(
    const std::vector<Vector3D<int>>& coords,
    i3d::Image3d<T>& /* dest */,
    const std::vector<Vector3D<int>>& /* offsets */) const {
	DatasetProperties props = get_dataset_properties(_ip, _port, _uuid);

	// TODO offload to details
	details::info("Checking validity of given coordinates");
	for (Vector3D<int> coord : coords)
		if (!details::is_block_coord_valid(coord, _resolution, props)) {
			details::error(
			    fmt::format("Block coordinate {} is out of valid range",
			                std::string(coord)));
			return false;
		}
	details::info("Check successfully finished");

	return {};
	// TODO
}

/* ===================================== Connection */

Connection::Connection(std::string ip, int port, std::string uuid)
    : _ip(std::move(ip)), _port(port), _uuid(std::move(uuid)) {}
} // namespace datastore
