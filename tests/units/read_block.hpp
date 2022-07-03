#pragma once

#include "../common.hpp"
#include <iostream>

namespace units {
template <typename T>
void test_read_block() {
	test_start("Read/Write block");

	phase_start("Fetch dataset properties");
	auto props = ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);
	phase_ok();

	phase_start("Available blocks computation");
	i3d::Vector3d<int> block_count = props.get_block_count(IMG_RESOLUTION);
	std::vector<i3d::Vector3d<int>> avail_coords;
	for (int x = 0; x < block_count.x; ++x)
		for (int y = 0; y < block_count.y; ++y)
			for (int z = 0; z < block_count.z; ++z)
				avail_coords.emplace_back(x, y, z);
	phase_ok();

	phase_start("Write-Read random blocks");
	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);
	ds::ImageView view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                   IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	for (auto coord : avail_coords) {
		auto block_size = props.get_block_size(coord, IMG_RESOLUTION);
		i3d::Image3d<T> random_block;
		random_block.MakeRoom(block_size);
		fill_random(random_block);

		view.write_block(random_block, coord, {0, 0, 0});

		auto view_block = view.read_block<T>(coord);
		auto conn_block =
		    conn.read_block<T>(coord, IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
		                       IMG_RESOLUTION, IMG_VERSION);

		assert(random_block == view_block);
		assert(random_block == conn_block);
	}
	phase_ok();

	phase_start("Read offsets");

	auto block_size = props.get_block_size({0, 0, 0}, IMG_RESOLUTION);
	i3d::Image3d<T> random_block;
	random_block.MakeRoom(block_size);
	fill_random(random_block);
	view.write_block(random_block, {0, 0, 0}, {0, 0, 0});

	i3d::Image3d<T> view_img;
	view_img.MakeRoom(block_size * 2);

	i3d::Image3d<T> conn_img;
	conn_img.MakeRoom(block_size * 2);

	for (int x = 0; x < block_size.x; x += block_size.x / 10)
		for (int y = 0; y < block_size.y; y += block_size.y / 10)
			for (int z = 0; z < block_size.z; z += block_size.z / 10) {
				i3d::Vector3d<int> offset{x, y, z};
				view.read_block({0, 0, 0}, view_img, offset);
				conn.read_block({0, 0, 0}, conn_img, offset, IMG_CHANNEL,
				                IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
				                IMG_VERSION);

				for (int xx = 0; xx < block_size.x; ++xx)
					for (int yy = 0; yy < block_size.y; ++yy)
						for (int zz = 0; zz < block_size.z; ++zz) {
							i3d::Vector3d<std::size_t> coord{std::size_t(xx),
							                                 std::size_t(yy),
							                                 std::size_t(zz)};
							assert(random_block.GetVoxel(coord) ==
							       view_img.GetVoxel(coord + offset));

							assert(random_block.GetVoxel(coord) ==
							       conn_img.GetVoxel(coord + offset));
						}
			}

	phase_ok();

	test_ok();
}
} // namespace units