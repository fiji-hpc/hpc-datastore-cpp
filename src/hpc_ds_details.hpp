#pragma once
#include "hpc_ds_structs.hpp"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <optional>
#include <source_location>
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

// TODO docs
inline std::string
get_dataset_url(const std::string& ip, int port, const std::string& uuid);

// TODO docs
inline void _log(const std::string& msg, const std::source_location& location);

// TODO docs
inline void
info(const std::string& msg,
     const std::source_location& location = std::source_location::current());

// TODO docs
inline void
warning(const std::string& msg,
        const std::source_location& location = std::source_location::current());

// TODO docs
inline void
error(const std::string& msg,
      const std::source_location& location = std::source_location::current());

// TODO docs
inline std::string
get_dataset_json_str(const std::string& ip, int port, const std::string& uuid);

// TODO docs
inline ::datastore::DatasetProperties
get_properties_from_json_str(const std::string& json_str);

namespace props_parser {
using namespace Poco::JSON;
using ::datastore::Vector3D;

// TODO finish implementations
std::string get_uuid(Object::Ptr root);
std::string get_voxel_type(Object::Ptr root);
Vector3D<int> get_dimensions(Object::Ptr root);
int get_channels(Object::Ptr root);
int get_angles(Object::Ptr root);
std::optional<std::string> get_transformations(Object::Ptr root);
std::string get_voxel_unit(Object::Ptr root);
Vector3D<double> get_voxel_resolution(Object::Ptr root);
std::optional<Vector3D<double>> get_timepoint_resolution(Object::Ptr root);
std::optional<Vector3D<double>> get_channel_resolution(Object::Ptr root);
std::optional<Vector3D<double>> get_angle_resolution(Object::Ptr root);
std::string get_compression(Object::Ptr root);

std::vector<std::map<std::string, Vector3D<int>>>
get_resolution_levels(Object::Ptr root);

std::vector<int> get_versions(Object::Ptr root);
std::string get_label(Object::Ptr root);
std::optional<std::string> get_view_registrations(Object::Ptr root);
std::vector<int> get_timepoint_ids(Object::Ptr root);

} // namespace props_parser
} // namespace details
} // namespace datastore

/* ================= IMPLEMENTATION FOLLOWS ======================== */
namespace datastore {
namespace details {

/* inline */ std::string
get_dataset_url(const std::string& ip, int port, const std::string& uuid) {
	if (!ip.starts_with("http://"))
		return fmt::format("https://{}:{}/datasets/{}", ip, port, uuid);
	return fmt::format("{}:{}/datasets/{}", ip, port, uuid);
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

	Poco::URI uri(dataset_url);
	std::string path(uri.getPathAndQuery());

	Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path,
	                               Poco::Net::HTTPMessage::HTTP_1_1);

	info("Sending HTTP-GET request to get dataset properties");
	session.sendRequest(request);

	Poco::Net::HTTPResponse response;
	std::istream& rs = session.receiveResponse(response);

	info(fmt::format("Fetched response with status: {}, reason: {}",
	                 response.getStatus(), response.getReason()));

	std::string out;
	rs >> out;
	return out;
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
	// UUID
	if (root->has("uuid"))
		props.uuid = root->getValue<std::string>("uuid");
	else
		warning("UUID was not found");

	// VoxelType
	if (root->has("voxelType"))
		props.voxel_type = root->getValue<std::string>("voxelType");
	else
		warning("voxelType was not found");

	// dimensions
	if (root->has("dimensions")) {
		Array::Ptr values = root->getArray("dimensions");
		if (values->size() == 3) {
			props.dimensions.x() = values->getElement<int>(0);
			props.dimensions.y() = values->getElement<int>(1);
			props.dimensions.z() = values->getElement<int>(2);
		} else
			warning("Incorrect number of dimensions");
	} else
		warning("dimensions were not found");

	/*
	props.uuid = get_uuid(root);
	props.voxel_type = get_voxel_type(root);
	props.dimensions = get_dimensions(root);
	props.channels = get_channels(root);
	props.angles = get_angles(root);
	props.transformations = get_transformations(root);
	props.voxel_unit = get_voxel_unit(root);
	props.voxel_resolution = get_voxel_resolution(root);
	props.timepoint_resolution = get_timepoint_resolution(root);
	props.channel_resolution = get_channel_resolution(root);
	props.angle_resolution = get_angle_resolution(root);
	props.compression = get_compression(root);
	props.resolution_levels = get_resolution_levels(root);
	props.versions = get_versions(root);
	props.label = get_label(root);
	props.view_registrations = get_view_registrations(root);
	props.timepoint_ids = get_timepoint_ids(root);
	*/

	info("Parsing has finished");
	return props;
}
} // namespace details
} // namespace datastore
