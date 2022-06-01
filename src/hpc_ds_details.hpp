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

// TODO finish implementations

template <Basic T>
T get_elem(Object::Ptr root, const std::string& name);

template <Vector3D_cnpt T>
T get_elem(Object::Ptr root, const std::string& name);

template <Vector_cnpt T>
T get_elem(Object::Ptr root, const std::string& name);

template <Optional_cnpt T>
T get_elem(Object::Ptr root, const std::string& name);

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

	props.uuid = get_elem<std::string>(root, "uuid");
	props.voxel_type = get_elem<std::string>(root, "voxelType");
	props.dimensions = get_elem<Vector3D<int>>(root, "dimensions");
	props.channels = get_elem<int>(root, "channels");
	props.angles = get_elem<int>(root, "angles");
	props.transformations =
	    get_elem<std::optional<std::string>>(root, "transformations");
	props.voxel_unit = get_elem<std::string>(root, "voxelUnit");
	props.voxel_resolution =
	    get_elem<std::optional<Vector3D<double>>>(root, "voxelResolution");
	props.timepoint_resolution =
	    get_elem<std::optional<Vector3D<double>>>(root, "timepointResolution");
	props.channel_resolution =
	    get_elem<std::optional<Vector3D<double>>>(root, "channelResolution");
	// props.angle_resolution = get_angle_resolution(root);
	props.compression = get_elem<std::string>(root, "compression");
	// props.resolution_levels = get_resolution_levels(root);
	props.versions = get_elem<std::vector<int>>(root, "versions");
	props.label = get_elem<std::string>(root, "label");
	// props.view_registrations = get_view_registrations(root);
	props.timepoint_ids = get_elem<std::vector<int>>(root, "timepointIds");

	info("Parsing has finished");
	return props;
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

template <Vector3D_cnpt T>
T get_elem(Object::Ptr root, const std::string& name) {
	using V = typename T::value_type;
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

} // namespace props_parser
} // namespace details
} // namespace datastore
