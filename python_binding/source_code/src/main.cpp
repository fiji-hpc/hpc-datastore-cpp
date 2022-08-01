#include "../../../src/hpc_ds_api.hpp"
#include "custom_casters.hpp"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace py::literals;

#define SELECT_TYPE_RV(func, type, ...)                                        \
	if (type == "uint8")                                                       \
		return func<uint8_t>(__VA_ARGS__);                                     \
	else if (type == "uint16")                                                 \
		return func<uint16_t>(__VA_ARGS__);                                    \
	else if (type == "float32")                                                \
		return func<float>(__VA_ARGS__);                                       \
	else                                                                       \
		throw py::type_error(                                                  \
		    "Only uint8, uint16 and float32 is currently supported \n");

#define SELECT_TYPE(func, type, ...)                                           \
	if (type == "uint8")                                                       \
		func<uint8_t>(__VA_ARGS__);                                            \
	else if (type == "uint16")                                                 \
		func<uint16_t>(__VA_ARGS__);                                           \
	else if (type == "float32")                                                \
		func<float>(__VA_ARGS__);                                              \
	else                                                                       \
		throw py::type_error(                                                  \
		    "Only uint8, uint16 and float32 is currently supported \n");

template <typename T>
void image3d_to_numpy(const i3d::Image3d<T>& img,
                      py::array_t<T>& dest,
                      i3d::Vector3d<int> dest_offset = {0, 0, 0}) {
	auto r = dest.template mutable_unchecked<3>();
	for (std::size_t x = 0; x < img.GetSizeX(); ++x)
		for (std::size_t y = 0; y < img.GetSizeY(); ++y)
			for (std::size_t z = 0; z < img.GetSizeZ(); ++z)
				r(x + dest_offset.x, y + dest_offset.y, z + dest_offset.z) =
				    img.GetVoxel(x, y, z);
}

template <typename T>
py::array_t<T> image3d_to_numpy(const i3d::Image3d<T>& img) {
	py::array_t<T> out({img.GetSizeX(), img.GetSizeY(), img.GetSizeZ()});
	image3d_to_numpy(img, out);
	return out;
}

template <typename T>
void numpy_to_image3d(const py::array_t<T>& img,
                      i3d::Image3d<T>& dest,
                      i3d::Vector3d<int> dest_offset = {0, 0, 0}) {
	auto r = img.template unchecked<3>();
	for (py::ssize_t x = 0; x < img.shape(0); ++x)
		for (py::ssize_t y = 0; y < img.shape(1); ++y)
			for (py::ssize_t z = 0; z < img.shape(2); ++z)
				dest.SetVoxel(x + dest_offset.x, y + dest_offset.y,
				              z + dest_offset.z, r(x, y, z));
}

template <typename T>
i3d::Image3d<T> numpy_to_image3d(const py::array_t<T>& img) {
	i3d::Image3d<T> out;
	out.MakeRoom(img.shape(0), img.shape(1), img.shape(2));

	numpy_to_image3d(img, out);
	return out;
}

/* TESING ENV <-- remove later */

/* END */

namespace pywrap {
template <typename T, typename... Params>
py::array read_image(Params... args) {
	auto img = ds::read_image<T>(args...);
	return image3d_to_numpy(img);
}

template <typename T, typename... Params>
void write_image(const py::array_t<T>& img, Params... args) {
	ds::write_image(numpy_to_image3d(img), args...);
}

template <typename T, typename... Params>
void write_with_pyramids(const py::array_t<T>& img, Params... args) {
	ds::write_with_pyramids(numpy_to_image3d(img), args...);
}

namespace ImageView {
template <typename T, typename... Params>
py::array read_block(ds::ImageView* view, Params... args) {
	auto img = view->read_block<T>(args...);
	return image3d_to_numpy(img);
}

template <typename T>
void read_block_inplace(ds::ImageView* view,
                        i3d::Vector3d<int> coord,
                        py::array_t<T> dest,
                        i3d::Vector3d<int> dest_offset) {
	auto img = view->read_block<T>(coord);
	image3d_to_numpy(img, dest, dest_offset);
}

template <typename T, typename... Params>
std::vector<py::array> read_blocks(ds::ImageView* view, Params... args) {
	auto imgs = view->read_blocks<T>(args...);
	std::vector<py::array> out;
	for (const auto& img : imgs)
		out.push_back(image3d_to_numpy(img));

	return out;
}

template <typename T>
void read_blocks_inplace(ds::ImageView* view,
                         const std::vector<i3d::Vector3d<int>>& coords,
                         py::array_t<T> dest,
                         const std::vector<i3d::Vector3d<int>>& offsets) {

	auto img = numpy_to_image3d(dest);
	view->read_blocks(coords, img, offsets);
	image3d_to_numpy(img, dest);
}

template <typename T, typename... Params>
py::array read_region(ds::ImageView* view, Params... args) {
	auto img = view->read_region<T>(args...);
	return image3d_to_numpy(img);
}

template <typename T>
void read_region_inplace(ds::ImageView* view,
                         i3d::Vector3d<int> start_point,
                         i3d::Vector3d<int> end_point,
                         py::array_t<T> dest,
                         i3d::Vector3d<int> offset) {

	auto reg = view->read_region<T>(start_point, end_point);
	image3d_to_numpy(reg, dest, offset);
}

template <typename T>
py::array read_image(ds::ImageView* view) {
	auto img = view->read_image<T>();
	return image3d_to_numpy(img);
}

template <typename T, typename... Params>
void write_block(ds::ImageView* view,
                 const py::array_t<T>& src,
                 Params... args) {
	view->write_block(numpy_to_image3d(src), args...);
}

template <typename T, typename... Params>
void write_blocks(ds::ImageView* view,
                  const py::array_t<T>& src,
                  Params... args) {
	view->write_blocks(numpy_to_image3d(src), args...);
}

template <typename T>
void write_image(ds::ImageView* view, const py::array_t<T>& src) {
	view->write_image(numpy_to_image3d(src));
}

} // namespace ImageView

namespace Connection {
template <typename T, typename... Params>
py::array read_block(ds::Connection* conn, Params... args) {
	auto img = conn->read_block<T>(args...);
	return image3d_to_numpy(img);
}

template <typename T, typename... Params>
void read_block_inplace(ds::Connection* conn,
                        i3d::Vector3d<int> coord,
                        py::array_t<T> dest,
                        i3d::Vector3d<int> dest_offset,
                        Params... args) {
	auto img = conn->read_block<T>(coord, args...);
	image3d_to_numpy(img, dest, dest_offset);
}

template <typename T, typename... Params>
std::vector<py::array> read_blocks(ds::Connection* conn, Params... args) {
	auto imgs = conn->read_blocks<T>(args...);
	std::vector<py::array> out;
	for (const auto& img : imgs)
		out.push_back(image3d_to_numpy(img));

	return out;
}

template <typename T, typename... Params>
void read_blocks_inplace(ds::Connection* conn,
                         const std::vector<i3d::Vector3d<int>>& coords,
                         py::array_t<T> dest,
                         Params... args) {

	auto img = numpy_to_image3d(dest);
	conn->read_blocks(coords, img, args...);
	image3d_to_numpy(img, dest);
}

template <typename T, typename... Params>
py::array read_region(ds::Connection* conn, Params... args) {
	auto img = conn->read_region<T>(args...);
	return image3d_to_numpy(img);
}

template <typename T, typename... Params>
void read_region_inplace(ds::Connection* conn,
                         i3d::Vector3d<int> start_point,
                         i3d::Vector3d<int> end_point,
                         py::array_t<T> dest,
                         i3d::Vector3d<int> offset,
                         Params... args) {

	auto reg = conn->read_region<T>(start_point, end_point, args...);
	image3d_to_numpy(reg, dest, offset);
}

template <typename T, typename... Params>
py::array read_image(ds::Connection* conn, Params... args) {
	auto img = conn->read_image<T>(args...);
	return image3d_to_numpy(img);
}

template <typename T, typename... Params>
void write_block(ds::Connection* conn,
                 const py::array_t<T>& src,
                 Params... args) {
	conn->write_block(numpy_to_image3d(src), args...);
}

template <typename T, typename... Params>
void write_blocks(ds::Connection* conn,
                  const py::array_t<T>& src,
                  Params... args) {
	conn->write_blocks(numpy_to_image3d(src), args...);
}

template <typename T, typename... Params>
void write_image(ds::Connection* conn,
                 const py::array_t<T>& src,
                 Params... args) {
	conn->write_image(numpy_to_image3d(src), args...);
}

template <typename T, typename... Params>
void write_with_pyramids(ds::Connection* conn,
                         const py::array_t<T>& src,
                         Params... args) {
	conn->write_with_pyramids(numpy_to_image3d(src), args...);
}

} // namespace Connection

} // namespace pywrap

PYBIND11_MODULE(hpc_datastore, m) {
	m.doc() = "HPC Datastore API python bind";

	using props_t = ds::DatasetProperties;
	py::class_<props_t, std::shared_ptr<props_t>>(m, "DatastoreProperties")
	    .def(py::init<>())
	    .def_readwrite("uuid", &props_t::uuid)
	    .def_readwrite("voxel_type", &props_t::voxel_type)
	    .def_readwrite("dimensions", &props_t::dimensions)
	    .def_readwrite("channels", &props_t::channels)
	    .def_readwrite("angles", &props_t::angles)
	    .def_readwrite("transformations", &props_t::transformations)
	    .def_readwrite("voxel_unit", &props_t::voxel_unit)
	    .def_readwrite("voxel_resolution", &props_t::voxel_resolution)
	    .def_readwrite("timepoint_resolution", &props_t::timepoint_resolution)
	    .def_readwrite("channel_resolution", &props_t::channel_resolution)
	    .def_readwrite("angle_resolution", &props_t::angle_resolution)
	    .def_readwrite("compression", &props_t::compression)
	    .def_readwrite("resolution_levels", &props_t::resolution_levels)
	    .def_readwrite("versions", &props_t::versions)
	    .def_readwrite("label", &props_t::label)
	    .def_readwrite("view_registrations", &props_t::view_registrations)
	    .def_readwrite("timepoint_ids", &props_t::timepoint_ids)
	    .def("get_block_dimensions", &props_t::get_block_dimensions)
	    .def("get_block_size", &props_t::get_block_size)
	    .def("get_block_count", &props_t::get_block_count)
	    .def("get_img_dimensions", &props_t::get_img_dimensions)
	    .def("get_all_resolutions", &props_t::get_all_resolutions)
	    .def("__str__", &props_t::str)
	    .def("__repr__", &props_t::str);

	py::enum_<ds::SamplingMode>(m, "SamplingMode")
	    .value("NEAREST_NEIGHBOUR", ds::SamplingMode::NEAREST_NEIGHBOUR)
	    .value("LINEAR", ds::SamplingMode::LINEAR)
	    .value("LANCZOS", ds::SamplingMode::LANCZOS);

	m.def("get_dataset_properties", &ds::get_dataset_properties, "ip"_a,
	      "port"_a, "uuid"_a);

	m.def(
	    "read_image",
	    [](const std::string& ip, int port, const std::string& uuid,
	       int channel, int timepoint, int angle, i3d::Vector3d<int> resolution,
	       const std::string& version) {
		    auto props = ds::get_dataset_properties(ip, port, uuid);
		    SELECT_TYPE_RV(pywrap::read_image, props->voxel_type, ip, port,
		                   uuid, channel, timepoint, angle, resolution,
		                   version);
	    },
	    "ip"_a, "port"_a, "uuid"_a, "channel"_a = 0, "timepoint"_a = 0,
	    "angle"_a = 0, "resolution"_a = i3d::Vector3d<int>{1, 1, 1},
	    "version"_a = "latest");

	m.def(
	    "write_image",
	    [](const py::array& img, const std::string& ip, int port,
	       const std::string& uuid, int channel, int timepoint, int angle,
	       i3d::Vector3d<int> resolution, const std::string& version) {
		    auto props = ds::get_dataset_properties(ip, port, uuid);
		    SELECT_TYPE_RV(pywrap::write_image, props->voxel_type, img, ip,
		                   port, uuid, channel, timepoint, angle, resolution,
		                   version);
	    },
	    "img"_a, "ip"_a, "port"_a, "uuid"_a, "channel"_a = 0, "timepoint"_a = 0,
	    "angle"_a = 0, "resolution"_a = i3d::Vector3d<int>{1, 1, 1},
	    "version"_a = "latest");

	m.def(
	    "write_with_pyramids",
	    [](const py::array& img, const std::string& ip, int port,
	       const std::string& uuid, int channel, int timepoint, int angle,
	       const std::string& version, ds::SamplingMode m) {
		    auto props = ds::get_dataset_properties(ip, port, uuid);
		    SELECT_TYPE_RV(pywrap::write_with_pyramids, props->voxel_type, img,
		                   ip, port, uuid, channel, timepoint, angle, version,
		                   m);
	    },
	    "img"_a, "ip"_a, "port"_a, "uuid"_a, "channel"_a = 0, "timepoint"_a = 0,
	    "angle"_a = 0, "version"_a = "latest",
	    "sampling_mode"_a = ds::SamplingMode::NEAREST_NEIGHBOUR);

	py::class_<ds::ImageView>(m, "ImageView")
	    .def(py::init<std::string, int, std::string, int, int, int,
	                  i3d::Vector3d<int>, std::string>(),
	         "ip"_a, "port"_a, "uuid"_a, "channel"_a, "timepoint"_a, "angle"_a,
	         "resolution"_a, "version"_a)
	    .def("get_properties", &ds::ImageView::get_properties)
	    .def(
	        "read_block",
	        [](ds::ImageView* self, i3d::Vector3d<int> coord) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::read_block, props->voxel_type,
		                       self, coord);
	        },
	        "coord"_a)
	    .def(
	        "read_block",
	        [](ds::ImageView* self, i3d::Vector3d<int> coord, py::array& dest,
	           i3d::Vector3d<int> dest_offset) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::read_block_inplace,
		                       props->voxel_type, self, coord, dest,
		                       dest_offset);
	        },
	        "coord"_a, "dest"_a, "dest_offset"_a = i3d::Vector3d<int>{0, 0, 0})
	    .def(
	        "read_blocks",
	        [](ds::ImageView* self,
	           const std::vector<i3d::Vector3d<int>>& coords) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::read_blocks,
		                       props->voxel_type, self, coords);
	        },
	        "coords"_a)
	    .def(
	        "read_blocks",
	        [](ds::ImageView* self,
	           const std::vector<i3d::Vector3d<int>>& coords, py::array& dest,
	           const std::vector<i3d::Vector3d<int>>& offsets) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::read_blocks_inplace,
		                       props->voxel_type, self, coords, dest, offsets);
	        },
	        "coords"_a, "dest"_a, "offsets"_a)
	    .def(
	        "read_region",
	        [](ds::ImageView* self, i3d::Vector3d<int> start_point,
	           i3d::Vector3d<int> end_point) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::read_region,
		                       props->voxel_type, self, start_point, end_point);
	        },
	        "start_point"_a, "end_point"_a)
	    .def(
	        "read_region",
	        [](ds::ImageView* self, i3d::Vector3d<int> start_point,
	           i3d::Vector3d<int> end_point, py::array& dest,
	           i3d::Vector3d<int> offset) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::read_region_inplace,
		                       props->voxel_type, self, start_point, end_point,
		                       dest, offset);
	        },
	        "start_point"_a, "end_point"_a, "dest"_a,
	        "offset"_a = i3d::Vector3d<int>{0, 0, 0})
	    .def("read_image",
	         [](ds::ImageView* self) {
		         auto props = self->get_properties();
		         SELECT_TYPE_RV(pywrap::ImageView::read_image,
		                        props->voxel_type, self);
	         })
	    .def(
	        "write_block",
	        [](ds::ImageView* self, const py::array& src,
	           i3d::Vector3d<int> coord, i3d::Vector3d<int> src_offset) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::write_block,
		                       props->voxel_type, self, src, coord, src_offset);
	        },
	        "src"_a, "coord"_a, "src_offset"_a = i3d::Vector3d<int>{0, 0, 0})
	    .def(
	        "write_blocks",
	        [](ds::ImageView* self, const py::array& src,
	           const std::vector<i3d::Vector3d<int>>& coords,
	           const std::vector<i3d::Vector3d<int>>& src_offsets) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::write_blocks,
		                       props->voxel_type, self, src, coords,
		                       src_offsets);
	        },
	        "src"_a, "coords"_a, "src_offsets"_a)
	    .def(
	        "write_image",
	        [](ds::ImageView* self, const py::array& src) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::ImageView::write_image,
		                       props->voxel_type, self, src);
	        },
	        "src"_a);

	py::class_<ds::Connection>(m, "Connection")
	    .def(py::init<std::string, int, std::string>(), "ip"_a, "port"_a,
	         "uuid"_a)
	    .def("get_view", &ds::Connection::get_view, "channel"_a, "timepoint"_a,
	         "angle"_a, "resolution"_a, "version"_a)
	    .def("get_properties", &ds::Connection::get_properties)
	    .def(
	        "read_block",
	        [](ds::Connection* self, i3d::Vector3d<int> coord, int channel,
	           int timepoint, int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_block,
		                       props->voxel_type, self, coord, channel,
		                       timepoint, angle, resolution, version);
	        },
	        "coord"_a, "channel"_a, "timepoint"_a, "angle"_a, "resolution"_a,
	        "version"_a)
	    .def(
	        "read_block",
	        [](ds::Connection* self, i3d::Vector3d<int> coord, py::array& dest,
	           i3d::Vector3d<int> dest_offset, int channel, int timepoint,
	           int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_block_inplace,
		                       props->voxel_type, self, coord, dest,
		                       dest_offset, channel, timepoint, angle,
		                       resolution, version);
	        },
	        "coord"_a, "dest"_a, "dest_offset"_a, "channel"_a, "timepoint"_a,
	        "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "read_blocks",
	        [](ds::Connection* self,
	           const std::vector<i3d::Vector3d<int>>& coords, int channel,
	           int timepoint, int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_blocks,
		                       props->voxel_type, self, coords, channel,
		                       timepoint, angle, resolution, version);
	        },
	        "coords"_a, "channel"_a, "timepoint"_a, "angle"_a, "resolution"_a,
	        "version"_a)
	    .def(
	        "read_blocks",
	        [](ds::Connection* self,
	           const std::vector<i3d::Vector3d<int>>& coords, py::array& dest,
	           const std::vector<i3d::Vector3d<int>>& offsets, int channel,
	           int timepoint, int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_blocks_inplace,
		                       props->voxel_type, self, coords, dest, offsets,
		                       channel, timepoint, angle, resolution, version);
	        },
	        "coords"_a, "dest"_a, "offsets"_a, "channel"_a, "timepoint"_a,
	        "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "read_region",
	        [](ds::Connection* self, i3d::Vector3d<int> start_point,
	           i3d::Vector3d<int> end_point, int channel, int timepoint,
	           int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_region,
		                       props->voxel_type, self, start_point, end_point,
		                       channel, timepoint, angle, resolution, version);
	        },
	        "start_point"_a, "end_point"_a, "channel"_a, "timepoint"_a,
	        "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "read_region",
	        [](ds::Connection* self, i3d::Vector3d<int> start_point,
	           i3d::Vector3d<int> end_point, py::array& dest,
	           i3d::Vector3d<int> offset, int channel, int timepoint, int angle,
	           i3d::Vector3d<int> resolution, const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_region_inplace,
		                       props->voxel_type, self, start_point, end_point,
		                       dest, offset, channel, timepoint, angle,
		                       resolution, version);
	        },
	        "start_point"_a, "end_point"_a, "dest"_a, "offset"_a, "channel"_a,
	        "timepoint"_a, "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "read_image",
	        [](ds::Connection* self, int channel, int timepoint, int angle,
	           i3d::Vector3d<int> resolution, const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::read_image,
		                       props->voxel_type, self, channel, timepoint,
		                       angle, resolution, version);
	        },
	        "channel"_a, "timepoint"_a, "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "write_block",
	        [](ds::Connection* self, const py::array& src,
	           i3d::Vector3d<int> coord, i3d::Vector3d<int> src_offset,
	           int channel, int timepoint, int angle,
	           i3d::Vector3d<int> resolution, const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::write_block,
		                       props->voxel_type, self, src, coord, src_offset,
		                       channel, timepoint, angle, resolution, version);
	        },
	        "src"_a, "coord"_a, "src_offset"_a, "channel"_a, "timepoint"_a,
	        "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "write_blocks",
	        [](ds::Connection* self, const py::array& src,
	           const std::vector<i3d::Vector3d<int>>& coords,
	           const std::vector<i3d::Vector3d<int>>& src_offsets, int channel,
	           int timepoint, int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::write_blocks,
		                       props->voxel_type, self, src, coords,
		                       src_offsets, channel, timepoint, angle,
		                       resolution, version);
	        },
	        "src"_a, "coords"_a, "src_offsets"_a, "channel"_a, "timepoint"_a,
	        "angle"_a, "resolution"_a, "version"_a)
	    .def(
	        "write_image",
	        [](ds::Connection* self, const py::array& src, int channel,
	           int timepoint, int angle, i3d::Vector3d<int> resolution,
	           const std::string& version) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::write_image,
		                       props->voxel_type, self, src, channel, timepoint,
		                       angle, resolution, version);
	        },
	        "src"_a, "channel"_a, "timepoint"_a, "angle"_a, "resolution"_a,
	        "version"_a)
	    .def(
	        "write_with_pyramids",
	        [](ds::Connection* self, const py::array& src, int channel,
	           int timepoint, int angle, const std::string& version,
	           ds::SamplingMode m) {
		        auto props = self->get_properties();
		        SELECT_TYPE_RV(pywrap::Connection::write_with_pyramids,
		                       props->voxel_type, self, src, channel, timepoint,
		                       angle, version, m);
	        },
	        "src"_a, "channel"_a, "timepoint"_a, "angle"_a, "version"_a,
	        "sampling_mode"_a);
}
