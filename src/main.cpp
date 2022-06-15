#include "hpc_ds_api.hpp"
#include <i3d/image3d.h>
#include <iostream>
#include <string>

const std::string ip = "http://127.0.0.1";
constexpr unsigned short port = 9080;
const std::string uuid = "156ddbf9-880d-4e17-aafd-f9e29db58c0d";

void compare() {
	i3d::Image3d<uint16_t> src("mask020.tif");
	i3d::Image3d<uint16_t> src2("mask.tif");

	for (std::size_t x = 0; x < src.GetSizeX(); ++x)
		for (std::size_t y = 0; y < src.GetSizeY(); ++y)
			for (std::size_t z = 0; z < src.GetSizeZ(); ++z)
				if (src.GetVoxel(x, y, z) != src2.GetVoxel(x, y, z))
					std::cout
					    << fmt::format("Unmatch: x={} y={} z={}\n", x, y, z);
}

int main() {
	datastore::ImageView view(ip, port, uuid, 0, 0, 0, {1, 1, 1}, "latest");

	/*
	i3d::Image3d<uint16_t> src("mask020.tif");
	view.write_image(src);
	*/

	std::cout << datastore::get_dataset_properties(ip, port, uuid) << '\n';

	auto img = view.read_image<uint16_t>();
	img.SaveImage("mask.tif");

	// compare();

	/*
	    i3d::Image3d<int> img;
	    img.MakeRoom(128, 64, 32);
	    img.SetVoxel(0, 0, 0, 71);
	    view.write_blocks(img, {{0, 0, 0}}, {{0, 0, 0}});

	    img.SetVoxel(0, 0, 0, 0);

	    view.read_blocks({{0, 0, 0}}, img, {{0, 0, 0}});
	    std::cout << fmt::format("Voxel value: {}\n", *img.GetVoxelAddr(0,
	   0, 0));
	*/
	std::cout << "Done" << '\n';
}

