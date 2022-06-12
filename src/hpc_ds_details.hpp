#pragma once
#include "hpc_ds_structs.hpp"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <i3d/image3d.h>
#include <i3d/vector3d.h>
#include <optional>
#include <source_location>
#include <span>
#include <string>
#include <type_traits>
/* ==================== DETAILS HEADERS ============================ */

namespace datastore {
namespace details {
#ifndef NDEBUG
constexpr inline bool debug = true;
#else
constexpr inline bool debug = false;
#endif

inline std::string
get_dataset_url(const std::string& ip, int port, const std::string& uuid);

inline void _log(const std::string& msg, const std::source_location& location);

inline void
info(const std::string& msg,
     const std::source_location& location = std::source_location::current());

inline void
warning(const std::string& msg,
        const std::source_location& location = std::source_location::current());

inline void
error(const std::string& msg,
      const std::source_location& location = std::source_location::current());

inline std::string
get_dataset_json_str(const std::string& ip, int port, const std::string& uuid);

inline DatasetProperties
get_properties_from_json_str(const std::string& json_str);

inline std::optional<i3d::Vector3d<int>>
get_block_dimensions(i3d::Vector3d<int> resolution,
                     const DatasetProperties& props);

inline bool check_block_coords(const std::vector<i3d::Vector3d<int>>& coords,
                               i3d::Vector3d<int> resolution,
                               const DatasetProperties& props);

template <typename T>
bool check_offset_coords(const std::vector<i3d::Vector3d<int>>& coords,
                         const i3d::Image3d<T>& img,
                         i3d::Vector3d<int> resolution,
                         const DatasetProperties& props);

inline int get_data_size(const DatasetProperties& props,
                         i3d::Vector3d<int> resolution);

inline int get_linear_index(i3d::Vector3d<int> coord,
                            i3d::Vector3d<int> block_dim,
                            const std::string& voxel_type);

template <typename T>
T get_elem_at(std::span<const char> data,
              const std::string& voxel_type,
              int index);

template <typename T>
T get_elem_at(std::span<const char> data,
              const std::string& voxel_type,
              i3d::Vector3d<int> block_dim,
              i3d::Vector3d<int> coord);

template <typename T>
void set_elem_at(std::span<char> data,
                 const std::string& voxel_type,
                 int index,
                 T elem);

template <typename T>
void set_elem_at(std::span<char> data,
                 const std::string& voxel_type,
                 i3d::Vector3d<int> block_dim,
                 i3d::Vector3d<int> coord,
                 T elem);

template <typename T>
void read_data(std::span<const char> data,
               const std::string& voxel_type,
               i3d::Image3d<T>& dest,
               i3d::Vector3d<int> offset);

template <typename T>
void write_data(const i3d::Image3d<T>& src,
                const std::string& voxel_type,
                i3d::Vector3d<int> block_dim,
                i3d::Vector3d<int> offset,
                std::span<char> data);

namespace props_parser {
using namespace Poco::JSON;

template <Basic T>
T get_elem(Object::Ptr root, const std::string& name);

template <Vector3d_cnpt T>
T get_elem(Object::Ptr root, const std::string& name);

template <Vector_cnpt T>
T get_elem(Object::Ptr root, const std::string& name);

template <Optional_cnpt T>
T get_elem(Object::Ptr root, const std::string& name);

inline std::vector<std::map<std::string, i3d::Vector3d<int>>>
get_resolution_levels(Object::Ptr root);

} // namespace props_parser

namespace requests {
inline std::string session_url_request(const std::string& ds_url,
                                       i3d::Vector3d<int> resolution,
                                       const std::string& version,
                                       access_mode mode);

inline std::pair<std::vector<char>, Poco::Net::HTTPResponse>
make_request(const std::string& url,
             const std::string& type = Poco::Net::HTTPRequest::HTTP_GET,
             const std::vector<char>& data = {},
             const std::map<std::string, std::string>& headers = {});
} // namespace requests

} // namespace details
} // namespace datastore

/* ================= IMPLEMENTATION FOLLOWS ======================== */
namespace datastore {
namespace details {

/* inline */ std::string
get_dataset_url(const std::string& ip, int port, const std::string& uuid) {
	std::string out;
	if (!ip.starts_with("http://"))
		out = "https://";
	return out + fmt::format("{}:{}/datasets/{}", ip, port, uuid);
}

/* inline */ void _log(const std::string& msg,
                       const std::string& type,
                       const std::source_location& location) {
	if constexpr (!debug)
		return;

	std::cout << fmt::format("[{}] {} at row {}:\n{} \n\n", type,
	                         location.function_name(), location.line(), msg);
	if (type.find("ERROR") != std::string::npos)
		std::cout << std::flush;
}

/* inline */ void info(const std::string& msg,
                       const std::source_location&
                           location /* = std::source_location::current() */) {
	_log(msg, "INFO", location);
}

/* inline */ void
warning(const std::string& msg,
        const std::source_location&
            location /* = std::source_location::current() */) {
	_log(msg, "WARNING", location);
}

/* inline */ void error(const std::string& msg,
                        const std::source_location&
                            location /* = std::source_location::current() */) {
	_log(msg, "ERROR", location);
}

/* inline */ std::string
get_dataset_json_str(const std::string& ip, int port, const std::string& uuid) {
	std::string dataset_url = get_dataset_url(ip, port, uuid);

	auto [data, response] = requests::make_request(dataset_url);
	int res_code = response.getStatus();

	if (res_code != 200)
		warning(fmt::format(
		    "Request returned with code: {}. json may be invalid", res_code));

	return std::string(data.begin(), data.end());
}

inline DatasetProperties
get_properties_from_json_str(const std::string& json_str) {

	using namespace Poco::JSON;
	info("Parsing dataset properties from JSON string");
	Parser parser;
	Poco::Dynamic::Var result = parser.parse(json_str);

	DatasetProperties props;
	auto root = result.extract<Object::Ptr>();

	using namespace props_parser;

	props.uuid = get_elem<std::string>(root, "uuid");
	props.voxel_type = get_elem<std::string>(root, "voxelType");
	props.dimensions = get_elem<i3d::Vector3d<int>>(root, "dimensions");
	props.channels = get_elem<int>(root, "channels");
	props.angles = get_elem<int>(root, "angles");
	props.transformations =
	    get_elem<std::optional<std::string>>(root, "transformations");
	props.voxel_unit = get_elem<std::string>(root, "voxelUnit");
	props.voxel_resolution =
	    get_elem<std::optional<i3d::Vector3d<double>>>(root, "voxelResolution");
	props.timepoint_resolution = get_elem<std::optional<i3d::Vector3d<double>>>(
	    root, "timepointResolution");
	props.channel_resolution = get_elem<std::optional<i3d::Vector3d<double>>>(
	    root, "channelResolution");
	props.angle_resolution =
	    get_elem<std::optional<i3d::Vector3d<double>>>(root, "angleResolution");
	props.compression = get_elem<std::string>(root, "compression");
	props.resolution_levels = get_resolution_levels(root);
	props.versions = get_elem<std::vector<int>>(root, "versions");
	props.label = get_elem<std::string>(root, "label");
	props.view_registrations =
	    get_elem<std::optional<std::string>>(root, "viewRegistrations");
	props.timepoint_ids = get_elem<std::vector<int>>(root, "timepointIds");

	info("Parsing has finished");
	return props;
}

/* inline */ std::optional<i3d::Vector3d<int>>
get_block_dimensions(i3d::Vector3d<int> resolution,
                     const DatasetProperties& props) {
	for (const auto& res_level : props.resolution_levels)
		if (res_level.at("resolutions") == resolution)
			return res_level.at("blockDimensions");

	error(fmt::format("Dimensions for resolution {} not found",
	                  vec_to_string(resolution)));
	return {};
}

/* inline */ bool
check_block_coords(const std::vector<i3d::Vector3d<int>>& coords,
                   i3d::Vector3d<int> resolution,
                   const DatasetProperties& props) {
	if constexpr (!debug)
		return true;

	details::info("Checking validity of given block coordinates");

	std::optional<i3d::Vector3d<int>> block_dims =
	    get_block_dimensions(resolution, props);

	if (!block_dims)
		return false;

	for (i3d::Vector3d<int> coord : coords)
		for (int i = 0; i < 3; ++i)
			if (coord[i] * block_dims.value()[i] >= props.dimensions[i]) {
				details::error(
				    fmt::format("Block coordinate {} is out of valid range",
				                vec_to_string(coord)));

				return false;
			}

	details::info("Check successfullly finished");
	return true;
}

template <typename T>
bool check_offset_coords(const std::vector<i3d::Vector3d<int>>& coords,
                         const i3d::Image3d<T>& img,
                         i3d::Vector3d<int> resolution,
                         const DatasetProperties& props) {
	if constexpr (!debug)
		return true;

	details::info("Checking validity of given offset coordinates");
	std::optional<i3d::Vector3d<int>> block_dims =
	    get_block_dimensions(resolution, props);

	if (!block_dims)
		return false;

	for (i3d::Vector3d<int> coord : coords)
		for (int i = 0; i < 3; ++i)
			if (img.GetSize()[i] <
			    std::size_t(coord[i] + block_dims.value()[i])) {
				details::error(
				    fmt::format("Offset coordinate {} is out of valid range",
				                vec_to_string(coord)));

				return false;
			}

	details::info("Check successfullly finished");
	return true;
}

/* inline */ int get_data_size(const DatasetProperties& props,
                               i3d::Vector3d<int> resolution) {
	std::optional<i3d::Vector3d<int>> block_dim =
	    get_block_dimensions(resolution, props);
	if (!block_dim)
		return -1;

	int elem_size = type_byte_size.at(props.voxel_type);
	return block_dim->x * block_dim->y * block_dim->z * elem_size + 12;
}

/* inline */ int get_linear_index(i3d::Vector3d<int> coord,
                                  i3d::Vector3d<int> block_dim,
                                  const std::string& voxel_type) {
	int elem_size = type_byte_size.at(voxel_type);

	return 12 +                                   // header_offset
	       (coord.x * block_dim.y * block_dim.z + // Main axis
	        coord.y * block_dim.z +               // secondary axis
	        coord.z) *                            // last axis
	           elem_size;                         // byte size
}

template <typename T>
T get_elem_at(std::span<const char> data,
              const std::string& voxel_type,
              int index) {
	int elem_size = type_byte_size.at(voxel_type);

	std::array<char, sizeof(T)> buffer{};
	std::copy_n(data.begin() + index,      // source start
	            elem_size,                 // count
	            buffer.end() - elem_size); // dest start

	std::ranges::reverse(buffer);

	return *reinterpret_cast<T*>(&buffer[0]);
}

template <typename T>
T get_elem_at(std::span<const char> data,
              const std::string& voxel_type,
              i3d::Vector3d<int> block_dim,
              i3d::Vector3d<int> coord) {

	int index = get_linear_index(coord, block_dim, voxel_type);
	return get_elem_at<T>(data, voxel_type, index);
}

template <typename T>
void set_elem_at(std::span<char> data,
                 const std::string& voxel_type,
                 int index,
                 T elem) {
	int elem_size = type_byte_size.at(voxel_type);

	auto buffer = *reinterpret_cast<std::array<char, sizeof(T)>*>(&elem);
	std::ranges::reverse(buffer);

	std::copy_n(buffer.end() - elem_size, // source start
	            elem_size,                // count
	            data.begin() + index);    // dest start
}

template <typename T>
void set_elem_at(std::span<char> data,
                 const std::string& voxel_type,
                 i3d::Vector3d<int> block_dim,
                 i3d::Vector3d<int> coord,
                 T elem) {
	int index = get_linear_index(coord, block_dim, voxel_type);
	set_elem_at(data, voxel_type, index, elem);
}

template <typename T>
void read_data(std::span<const char> data,
               const std::string& voxel_type,
               i3d::Image3d<T>& dest,
               i3d::Vector3d<int> offset) {
	i3d::Vector3d<int> block_dim;
	for (int i = 0; i < 3; ++i)
		block_dim[i] = get_elem_at<int>(data, "uint32", i * 4);

	for (int x = 0; x < block_dim.x; ++x)
		for (int y = 0; y < block_dim.y; ++y)
			for (int z = 0; z < block_dim.z; ++z)
				dest.SetVoxel(
				    x + offset.x, y + offset.y, z + offset.z,
				    get_elem_at<T>(data, voxel_type, block_dim, {x, y, z}));
}

template <typename T>
void write_data(const i3d::Image3d<T>& src,
                const std::string& voxel_type,
                i3d::Vector3d<int> block_dim,
                i3d::Vector3d<int> offset,
                std::span<char> data) {
	set_elem_at(data, "uint32", 0, block_dim.x);
	set_elem_at(data, "uint32", 4, block_dim.y);
	set_elem_at(data, "uint32", 8, block_dim.z);

	for (int x = 0; x < block_dim.x; ++x)
		for (int y = 0; y < block_dim.y; ++y)
			for (int z = 0; z < block_dim.z; ++z)
				set_elem_at(data, voxel_type, block_dim, {x, y, z},
				            *src.GetVoxelAddr(x + offset.x, y + offset.y,
				                              z + offset.z));
}

namespace props_parser {

template <Basic T>
T get_elem(Object::Ptr root, const std::string& name) {
	if (!root->has(name)) {
		warning(fmt::format("{} was not found", name));
		return {};
	}
	return root->getValue<T>(name);
}

template <Vector3d_cnpt T>
T get_elem(Object::Ptr root, const std::string& name) {
	using V = decltype(T{}.x);
	if (!root->has(name)) {
		warning(fmt::format("{} were not found", name));
		return {};
	}

	Array::Ptr values = root->getArray(name);
	if (values->size() != 3) {
		warning("Incorrect number of dimensions");
		return {};
	}

	T out;
	for (unsigned i = 0; i < 3; ++i)
		out[i] = values->getElement<V>(i);

	return out;
}

template <Vector_cnpt T>
T get_elem(Object::Ptr root, const std::string& name) {
	using V = typename T::value_type;
	if (!root->has(name)) {
		warning(fmt::format("{} were not found", name));
		return {};
	}

	Array::Ptr values = root->getArray(name);
	std::size_t count = values->size();

	T out(count);
	for (unsigned i = 0; i < count; ++i)
		out[i] = values->getElement<V>(i);

	return out;
}

template <Optional_cnpt T>
T get_elem(Object::Ptr root, const std::string& name) {
	if (!root->has(name)) {
		warning(fmt::format("{} were not found", name));
		return {};
	}

	if (root->isNull(name))
		return {};

	T out;
	out = get_elem<typename T::value_type>(root, name);
	return out;
}

/* inline */ std::vector<std::map<std::string, i3d::Vector3d<int>>>
get_resolution_levels(Object::Ptr root) {
	std::string name = "resolutionLevels";

	if (!root->has(name)) {
		warning("resolutionLevels were not found");
		return {};
	}

	std::vector<std::map<std::string, i3d::Vector3d<int>>> out;

	Array::Ptr array = root->getArray(name);
	for (unsigned i = 0; i < array->size(); ++i) {
		std::map<std::string, i3d::Vector3d<int>> map;
		Object::Ptr map_ptr = array->getObject(i);

		for (const auto& name : map_ptr->getNames()) {
			map[name] = get_elem<i3d::Vector3d<int>>(map_ptr, name);
		}

		out.push_back(map);
	}

	return out;
}

} // namespace props_parser

namespace requests {
/* inline */ std::string session_url_request(const std::string& ds_url,
                                             i3d::Vector3d<int> resolution,
                                             const std::string& version,
                                             access_mode mode) {

	std::string mode_str = mode_to_string(mode);

	info(fmt::format(
	    "Obtaining session url for resolution: {}, version: {}, mode: {}",
	    vec_to_string(resolution), version, mode_str));
	std::string req_url =
	    fmt::format("{}/{}/{}/{}/{}/{}", ds_url, resolution.x, resolution.y,
	                resolution.z, version, mode_str);

	auto [_, response] = make_request(req_url);

	int res_code = response.getStatus();
	if (res_code != 307)
		warning(fmt::format(
		    "Request ended with status: {}, redirection may be incorrect",
		    res_code));

	return response.get("Location");
}

/* inline */ std::pair<std::vector<char>, Poco::Net::HTTPResponse>
make_request(const std::string& url,
             const std::string& type /*  = Poco::Net::HTTPRequest::HTTP_GET */,
             const std::vector<char>& data /*  = {} */,
             const std::map<std::string, std::string>& headers /* = {} */) {
	Poco::URI uri(url);
	std::string path(uri.getPathAndQuery());

	Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

	Poco::Net::HTTPRequest request(type, path,
	                               Poco::Net::HTTPMessage::HTTP_1_1);

	for (auto& [key, value] : headers)
		request.set(key, value);

	request.setContentLength(data.size());

	info(fmt::format("Sending {} request to url: {}", type, url));
	std::ostream& os = session.sendRequest(request);
	for (char ch : data)
		os << ch;

	Poco::Net::HTTPResponse response;
	std::istream& rs = session.receiveResponse(response);

	std::vector<char> out{std::istreambuf_iterator<char>(rs),
	                      std::istreambuf_iterator<char>()};

	info(fmt::format(
	    "Fetched response with status: {}, reason: {}, content size: {}",
	    response.getStatus(), response.getReason(), out.size()));

	return {out, response};
}

} // namespace requests
} // namespace details
} // namespace datastore
