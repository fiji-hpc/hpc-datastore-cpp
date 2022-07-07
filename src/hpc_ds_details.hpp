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

namespace ds {
namespace details {
/* Definition of compile settings */

#ifdef DATASTORE_NDEBUG
constexpr inline bool _DEBUG_ = false;
#else
#ifdef NDEBUG
constexpr inline bool _DEBUG_ = false;
#else
constexpr inline bool _DEBUG_ = true;
#endif
#endif

#ifdef DATASTORE_NLOG
constexpr inline bool _LOG_ = false;
#else
constexpr inline bool _LOG_ = _DEBUG_;
#endif

#ifdef DATASTORE_NINFO
constexpr inline bool _INFO_ = false;
#else
constexpr inline bool _INFO_ = _LOG_;
#endif

#ifdef DATASTORE_NWARNING
constexpr inline bool _WARNING_ = false;
#else
constexpr inline bool _WARNING_ = _LOG_;
#endif

/**
 * @brief Get the dataset url objimagect
 *
 * @param ip ip address of the server
 * @param port port, where the server is listening on
 * @param uuid unique identifier of dataset
 * @return std::string dataset url
 */
inline std::string
get_dataset_url(const std::string& ip, int port, const std::string& uuid);

/**
 * @brief Get the dataset properties
 *
 * @param dataset_url
 * @return DatasetProperties
 */
inline DatasetProperties get_dataset_properties(const std::string& dataset_url);

/**
 * @brief Check consistency of block coordinates with respect to server
 *
 * Returns always 'true'  when not in debug
 *
 * @param coords Coordinates
 * @param img_dim image dimensions for requested resolution
 * @param block_dim dimension of block for requested resolution
 * @return true At no error
 * @return false At error (ERROR is emmited)
 */
inline bool check_block_coords(const std::vector<i3d::Vector3d<int>>& coords,
                               i3d::Vector3d<int> img_dim,
                               i3d::Vector3d<int> block_dim);

inline std::vector<i3d::Vector3d<int>>
get_intercepted_blocks(i3d::Vector3d<int> start_point,
                       i3d::Vector3d<int> end_point,
                       i3d::Vector3d<int> img_dim,
                       i3d::Vector3d<int> block_dim);

/**
 * @brief Create an optimized requests
 *
 * @param coords requested block coordinates
 * @param session_url connection session url
 * @param timepoint
 * @param channel
 * @param angle
 * @return Vector of pair: {request_url, coord indexes}
 */
inline std::vector<std::pair<std::string, std::vector<std::size_t>>>
create_requests(const std::vector<i3d::Vector3d<int>>& coords,
                const std::string& session_url,
                int timepoint,
                int channel,
                int angle,
                std::size_t max_request_size = MAX_URL_LENGTH);

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

/**
 * @brief Read data to image
 *
 * @tparam T Backend type of image
 * @param data octet-data to read from
 * @param voxel_type data type of image in <data>
 * @param dest destination image
 * @param offset offset to destination image
 */
template <typename T>
void read_data(std::span<const char> data,
               const std::string& voxel_type,
               i3d::Image3d<T>& dest,
               i3d::Vector3d<int> offset);

/**
 * @brief Write image to data
 *
 * @tparam T Backend type of image
 * @param src source image
 * @param offset offset to source image
 * @param data preallocated octet-data (ensure proper size)
 * @param voxel_type data type of image in <data>
 * @param block_size regular block size
 */
template <typename T>
void write_data(const i3d::Image3d<T>& src,
                i3d::Vector3d<int> offset,
                std::span<char> data,
                const std::string& voxel_type,
                i3d::Vector3d<int> block_size);
} // namespace data_manip

namespace log {

/**
 * @brief Prints message
 *
 * @param msg Message
 * @param type Type of message
 * @param location Location info about message source
 */
inline void _log(const std::string& msg,
                 const std::string& type,
                 const std::source_location& location);

/**
 * @brief Print info message
 *
 * @param msg Message
 * @param location Automaticaly generated location info
 */
inline void
info(const std::string& msg,
     const std::source_location& location = std::source_location::current());

/**
 * @brief Print warning message
 *
 * @param msg Message
 * @param location Automaticaly generated location info
 */
inline void
warning(const std::string& msg,
        const std::source_location& location = std::source_location::current());

} // namespace log
/* Helpers to parse Dataset Properties from JSON */
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

/* Helpers providing requests functionality */
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
} // namespace ds

/* ================= IMPLEMENTATION FOLLOWS ======================== */
namespace ds {
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

	/* Fetch JSON from server */
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

	/* Parse elements from JSON */

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

/* inline */ bool
check_block_coords(const std::vector<i3d::Vector3d<int>>& coords,
                   i3d::Vector3d<int> img_dim,
                   i3d::Vector3d<int> block_dim) {
	/* Act as NOOP if not in debug */
	if constexpr (!_DEBUG_)
		return true;

	log::info("Checking validity of given block coordinates");

	for (i3d::Vector3d<int> coord : coords)
		if (data_manip::get_block_size(coord, block_dim, img_dim) ==
		    i3d::Vector3d(0, 0, 0)) {
			log::warning(fmt::format(
			    "Block coordinate {} is out of valid range", to_string(coord)));

			return false;
		}
	log::info("Check successfullly finished");
	return true;
}

/* inline */ std::vector<i3d::Vector3d<int>>
get_intercepted_blocks(i3d::Vector3d<int> start_point,
                       i3d::Vector3d<int> end_point,
                       i3d::Vector3d<int> img_dim,
                       i3d::Vector3d<int> block_dim) {
	assert(lt(start_point, end_point));

	i3d::Vector3d<int> block_count = (img_dim + block_dim - 1) / block_dim;
	std::vector<i3d::Vector3d<int>> out;

	for (int x = 0; x < block_count.x; ++x)
		for (int y = 0; y < block_count.y; ++y)
			for (int z = 0; z < block_count.z; ++z) {
				i3d::Vector3d<int> coord = {x, y, z};
				if (lt(start_point, (coord + 1) * block_dim) &&
				    lt(coord * block_dim, end_point))
					out.push_back(coord);
			}

	return out;
}

/* inline */ std::vector<std::pair<std::string, std::vector<std::size_t>>>
create_requests(const std::vector<i3d::Vector3d<int>>& coords,
                const std::string& session_url,
                int timepoint,
                int channel,
                int angle,
                std::size_t max_request_size /* = MAX_URL_LENGTH*/) {
	std::vector<std::pair<std::string, std::vector<std::size_t>>> out;

	std::string final_url = session_url;
	std::vector<std::size_t> indexes;

	for (std::size_t i = 0; i < coords.size(); ++i) {
		const auto& coord = coords[i];
		std::string to_append =
		    fmt::format("/{}/{}/{}/{}/{}/{}", coord.x, coord.y, coord.z,
		                timepoint, channel, angle);

		if (final_url.size() + to_append.size() > max_request_size) {
			out.emplace_back(final_url, indexes);
			final_url = session_url;
			indexes.clear();
		}

		final_url += to_append;
		indexes.push_back(i);
	}

	if (!indexes.empty()) {
		out.emplace_back(final_url, indexes);
	}

	return out;
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
			for (int z = 0; z < block_dim.z; ++z) {
				i3d::Vector3d<int> coord{x + offset.x, y + offset.y,
				                         z + offset.z};

				if (!(0 <= coord.x && coord.x < int(dest.GetSizeX())) ||
				    !(0 <= coord.y && coord.y < int(dest.GetSizeY())) ||
				    !(0 <= coord.z && coord.z < int(dest.GetSizeZ())))
					continue;

				dest.SetVoxel(coord, get_elem_at<T>(data, voxel_type, {x, y, z},
				                                    block_dim));
			}
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
	if constexpr (!_LOG_)
		return;

	std::cout << fmt::format("[{}] {} at row {}:\n{} \n\n", type,
	                         location.function_name(), location.line(), msg);
}

/* inline */ void info(const std::string& msg,
                       const std::source_location&
                           location /* = std::source_location::current() */) {
	if constexpr (!_INFO_)
		return;
	_log(msg, "INFO", location);
}

/* inline */ void
warning(const std::string& msg,
        const std::source_location&
            location /* = std::source_location::current() */) {
	if constexpr (!_WARNING_)
		return;
	_log(msg, "WARNING", location);
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
} // namespace ds
