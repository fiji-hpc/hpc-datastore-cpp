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

	phase_start("Offsets");

    phase_ok();

	test_ok();
}
} // namespace units