#include "hpc_ds_api.hpp"
#include <i3d/image3d.h>
#include <iostream>
#include <string>

const std::string ip = "http://127.0.0.1";
constexpr unsigned short port = 9080;
const std::string uuid = "64aec83b-332e-45a6-9aed-0051d7bb2cf8";

int main() {
	datastore::ImageView view(ip, port, uuid, 0, 0, 0, {1, 1, 1}, "latest");

	i3d::Image3d<uint16_t> src("mask020.tif");
	view.write_image(src);

	auto img = view.read_image<uint16_t>();
	img.SaveImage("mask.tif");
	/*
	    i3d::Image3d<int> img;
	    img.MakeRoom(128, 64, 32);
	    img.SetVoxel(0, 0, 0, 71);
	    view.write_blocks(img, {{0, 0, 0}}, {{0, 0, 0}});

	    img.SetVoxel(0, 0, 0, 0);

	    view.read_blocks({{0, 0, 0}}, img, {{0, 0, 0}});
	    std::cout << fmt::format("Voxel value: {}\n", *img.GetVoxelAddr(0, 0,
	   0));

	    std::cout << "Done" << '\n';
	*/
}

