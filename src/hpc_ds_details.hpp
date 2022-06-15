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
#ifdef DATASTORE_NDEBUG 
constexpr inline bool debug = false;
#else
#ifdef NDEBUG
constexpr inline bool debug = false;
#else 
constexpr inline bool debug = true;
#endif
#endif

inline std::string
get_dataset_url(const std::string& ip, int port, const std::string& uuid);

inline DatasetProperties get_dataset_properties(const std::string& dataset_url);

inline i3d::Vector3d<int> get_block_dimensions(const DatasetProperties& props,
                                               i3d::Vector3d<int> resolution);

inline bool check_block_coords(const std::vector<i3d::Vector3d<int>>& coords,
                               i3d::Vector3d<int> img_dim,
                               i3d::Vector3d<int> block_dim);

template <typename T>
bool check_offset_coords(const std::vector<i3d::Vector3d<int>>& offsets,
                         const std::vector<i3d::Vector3d<int>>& coords,
                         const i3d::Image3d<T>& img,
                         i3d::Vector3d<int> block_dim,
                         i3d::Vector3d<int> img_dim);
namespace data_manip {
inline int get_block_data_size(i3d::Vector3d<int> block_size,
                               const std::string& voxel_type);

inline i3d::Vector3d<int> get_block_size(i3d::Vector3d<int> coord,
                                         i3d::Vector3d<int> block_dim,
                                         i3d::Vector3d<int> img_dim);

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
              i3d::Vector3d<int> coord,
              i3d::Vector3d<int> block_dim);

template <typename T>
void set_elem_at(std::span<char> data,
                 const std::string& voxel_type,
                 int index,
                 T elem);

template <typename T>
void set_elem_at(std::span<char> data,
                 const std::string& voxel_type,
                 i3d::Vector3d<int> coord,
                 i3d::Vector3d<int> block_dim,
                 T elem);

template <typename T>
void read_data(std::span<const char> data,
               const std::string& voxel_type,
               i3d::Image3d<T>& dest,
               i3d::Vector3d<int> offset);

template <typename T>
void write_data(const i3d::Image3d<T>& src,
                i3d::Vector3d<int> offset,
                std::span<char> data,
                const std::string& voxel_type,
                i3d::Vector3d<int> block_size);
} // namespace data_manip

namespace log {
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
} // namespace log

namespace props_parser {
using namespace Poco::JSON;

template <cnpts::Basic T>
T get_elem(Object::Ptr root, const std::string& name);

template <cnpts::Vector3d T>
T get_elem(Object::Ptr root, const std::string& name);

template <cnpts::Vector T>
T get_elem(Object::Ptr root, const std::string& name);

template <cnpts::ResolutionUnit T>
T get_elem(Object::Ptr root, const std::string& name);

template <cnpts::Optional T>
T get_elem(Object::Ptr root, const std::string& name);

inline std::vector<std::map<std::string, i3d::Vector3d<int>>>
get_resolution_levels(Object::Ptr root);

} // namespace props_parser

namespace requests {
inline std::string session_url_request(const std::string& ds_url,
                                       i3d::Vector3d<int> resolution,
                                       const std::string& version);

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

inline DatasetProperties
get_dataset_properties(const std::string& dataset_url) {
	using namespace Poco::JSON;

	auto [data, response] = requests::make_request(dataset_url);
	std::string json_str(data.begin(), data.end());

	int res_code = response.getStatus();
	if (res_code != 200)
		log::warning(fmt::format(
		    "Request ended with code: {}. json may not be valid", res_code));

	log::info("Parsing dataset properties from JSON string");
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
	props.timepoint_resolution =
	    get_elem<std::optional<ResolutionUnit>>(root, "timepointResolution");
	props.channel_resolution =
	    get_elem<std::optional<ResolutionUnit>>(root, "channelResolution");
	props.angle_resolution =
	    get_elem<std::optional<ResolutionUnit>>(root, "angleResolution");
	props.compression = get_elem<std::string>(root, "compression");
	props.resolution_levels = get_resolution_levels(root);
	props.versions = get_elem<std::vector<int>>(root, "versions");
	props.label = get_elem<std::string>(root, "label");
	props.view_registrations =
	    get_elem<std::optional<std::string>>(root, "viewRegistrations");
	props.timepoint_ids = get_elem<std::vector<int>>(root, "timepointIds");

	log::info("Parsing has finished");
	return props;
}

/* inline */ i3d::Vector3d<int>
get_block_dimensions(const DatasetProperties& props,
                     i3d::Vector3d<int> resolution) {
	for (const auto& res_level : props.resolution_levels)
		if (res_level.at("resolutions") == resolution)
			return res_level.at("blockDimensions");

	log::error(fmt::format("Dimensions for resolution {} not found",
	                       to_string(resolution)));
	return {-1, -1, -1};
}

/* inline */ bool
check_block_coords(const std::vector<i3d::Vector3d<int>>& coords,
                   i3d::Vector3d<int> img_dim,
                   i3d::Vector3d<int> block_dim) {
	if constexpr (!debug)
		return true;

	log::info("Checking validity of given block coordinates");

	for (i3d::Vector3d<int> coord : coords)
		if (data_manip::get_block_size(coord, block_dim, img_dim) ==
		    i3d::Vector3d(0, 0, 0)) {
			log::error(fmt::format("Block coordinate {} is out of valid range",
			                       to_string(coord)));

			return false;
		}
	log::info("Check successfullly finished");
	return true;
}

template <typename T>
bool check_offset_coords(const std::vector<i3d::Vector3d<int>>& offsets,
                         const std::vector<i3d::Vector3d<int>>& coords,
                         const i3d::Image3d<T>& img,
                         i3d::Vector3d<int> block_dim,
                         i3d::Vector3d<int> img_dim) {
	if constexpr (!debug)
		return true;

	if (offsets.size() != coords.size())
		return false;

	log::info("Checking validity of given offset coordinates");

	for (std::size_t i = 0; i < coords.size(); ++i) {
		auto& coord = coords[i];
		auto& offset = offsets[i];

		i3d::Vector3d<int> block_size =
		    data_manip::get_block_size(coord, block_dim, img_dim);
		for (int i = 0; i < 3; ++i)
			if (!(0 <= coord[i] &&
			      std::size_t(offset[i] + block_size[i]) <= img.GetSize()[i])) {
				log::error(
				    fmt::format("Offset coordinate {} is out of valid range",
				                to_string(coord)));

				return false;
			}
	}
	log::info("Check successfullly finished");
	return true;
}

namespace data_manip {
/* inline */ int get_block_data_size(i3d::Vector3d<int> block_size,
                                     const std::string& voxel_type) {

	int elem_size = type_byte_size.at(voxel_type);
	return block_size.x * block_size.y * block_size.z * elem_size + 12;
}

/* inline */ i3d::Vector3d<int> get_block_size(i3d::Vector3d<int> coord,
                                               i3d::Vector3d<int> block_dim,
                                               i3d::Vector3d<int> img_dim) {
	i3d::Vector3d<int> start = (coord * block_dim);
	i3d::Vector3d<int> end = (coord + 1) * block_dim;

	i3d::Vector3d<int> out;
	for (int i = 0; i < 3; ++i) {
		out[i] =
		    std::max(0, std::min(img_dim[i], end[i]) - std::max(start[i], 0));
	}
	return out;
}

/* inline */ int get_linear_index(i3d::Vector3d<int> coord,
                                  i3d::Vector3d<int> block_dim,
                                  const std::string& voxel_type) {
	int elem_size = type_byte_size.at(voxel_type);

	return 12 +                                   // header_offset
	       (coord.z * block_dim.x * block_dim.y + // Main axis
	        coord.y * block_dim.x +               // secondary axis
	        coord.x) *                            // last axis
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
              i3d::Vector3d<int> coord,
              i3d::Vector3d<int> block_dim) {

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
                 i3d::Vector3d<int> coord,
                 i3d::Vector3d<int> block_dim,
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
				    get_elem_at<T>(data, voxel_type, {x, y, z}, block_dim));
}

template <typename T>
void write_data(const i3d::Image3d<T>& src,
                i3d::Vector3d<int> offset,
                std::span<char> data,
                const std::string& voxel_type,
                i3d::Vector3d<int> block_size) {
	set_elem_at(data, "uint32", 0, block_size.x);
	set_elem_at(data, "uint32", 4, block_size.y);
	set_elem_at(data, "uint32", 8, block_size.z);

	for (int x = 0; x < block_size.x; ++x)
		for (int y = 0; y < block_size.y; ++y)
			for (int z = 0; z < block_size.z; ++z)
				set_elem_at(
				    data, voxel_type, {x, y, z}, block_size,
				    src.GetVoxel(x + offset.x, y + offset.y, z + offset.z));
}
} // namespace data_manip

namespace log {
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
} // namespace log

namespace props_parser {

template <cnpts::Basic T>
T get_elem(Object::Ptr root, const std::string& name) {
	if (!root->has(name)) {
		log::warning(fmt::format("{} was not found", name));
		return {};
	}
	return root->getValue<T>(name);
}

template <cnpts::Vector3d T>
T get_elem(Object::Ptr root, const std::string& name) {
	using V = decltype(T{}.x);
	if (!root->has(name)) {
		log::warning(fmt::format("{} were not found", name));
		return {};
	}

	Array::Ptr values = root->getArray(name);
	if (values->size() != 3) {
		log::warning("Incorrect number of dimensions");
		return {};
	}

	T out;
	for (unsigned i = 0; i < 3; ++i)
		out[i] = values->getElement<V>(i);

	return out;
}

template <cnpts::Vector T>
T get_elem(Object::Ptr root, const std::string& name) {
	using V = typename T::value_type;
	if (!root->has(name)) {
		log::warning(fmt::format("{} were not found", name));
		return {};
	}

	Array::Ptr values = root->getArray(name);
	std::size_t count = values->size();

	T out(count);
	for (unsigned i = 0; i < count; ++i)
		out[i] = values->getElement<V>(i);

	return out;
}

template <cnpts::ResolutionUnit T>
T get_elem(Object::Ptr root, const std::string& name) {
	if (!root->has(name)) {
		log::warning(fmt::format("{} was not found", name));
		return {};
	}

	Object::Ptr res_ptr = root->getObject(name);
	ResolutionUnit res;

	if (res_ptr->has("value")) {
		res.value = res_ptr->getValue<double>("value");
	}

	if (res_ptr->has("unit")) {
		res.unit = res_ptr->getValue<std::string>("unit");
	}

	return res;
}

template <cnpts::Optional T>
T get_elem(Object::Ptr root, const std::string& name) {
	if (!root->has(name)) {
		log::warning(fmt::format("{} were not found", name));
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
		log::warning("resolutionLevels were not found");
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
                                             const std::string& version) {

	log::info(
	    fmt::format("Obtaining session url for resolution: {}, version: {}",
	                to_string(resolution), version));
	std::string req_url =
	    fmt::format("{}/{}/{}/{}/{}/read-write", ds_url, resolution.x,
	                resolution.y, resolution.z, version);

	auto [_, response] = make_request(req_url);

	int res_code = response.getStatus();
	if (res_code != 307)
		log::warning(fmt::format(
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

	log::info(fmt::format("Sending {} request to url: {}", type, url));
	std::ostream& os = session.sendRequest(request);
	for (char ch : data)
		os << ch;

	Poco::Net::HTTPResponse response;
	std::istream& rs = session.receiveResponse(response);

	std::vector<char> out{std::istreambuf_iterator<char>(rs),
	                      std::istreambuf_iterator<char>()};

	log::info(fmt::format(
	    "Fetched response with status: {}, reason: {}, content size: {}",
	    response.getStatus(), response.getReason(), out.size()));

	return {out, response};
}

} // namespace requests
} // namespace details
} // namespace datastore
