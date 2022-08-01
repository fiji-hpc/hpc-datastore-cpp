#include "../../../src/hpc_ds_api.hpp"
#include "custom_casters.hpp"
#include "wrappers.hpp"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

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
