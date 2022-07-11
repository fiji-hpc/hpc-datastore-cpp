#include "../../../src/hpc_ds_api.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace py::literals;

std::unique_ptr<ds::DatasetProperties> foo(const std::string&, int, const std::string&)
{
	return std::make_unique<ds::DatasetProperties>();
}

PYBIND11_MODULE(hpc_datastore, m) {
	m.doc() = "HPC Datastore API python bind";

	using props_t = ds::DatasetProperties;
	py::class_<props_t>(m, "DatastoreProperties")
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

	m.def("get_dataset_properties", &ds::get_dataset_properties, "ip"_a, "port"_a, "uuid"_a);
}
