#pragma once

#include <i3d/image3d.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace py::literals;

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