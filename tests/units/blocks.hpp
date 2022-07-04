#pragma once

#include "../common.hpp"
#include <iostream>

namespace units {
template <typename T>
void test_blocks() {
	test_start("Read/Write blocks");

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

	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);
	ds::ImageView view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                   IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	phase_start("Generating random blocks");

	std::vector<i3d::Vector3d<int>> shuffled = avail_coords;
	shuffle(shuffled);
	std::vector<i3d::Image3d<T>> view_random_blocks;
	std::vector<i3d::Image3d<T>> conn_random_blocks;

	for (auto coord : shuffled) {
		auto emplace_block = [&](auto& vec) {
			vec.emplace_back();
			auto& img = vec.back();

			img.MakeRoom(props.get_block_size(coord, IMG_RESOLUTION));
			fill_random(img);
		};

		emplace_block(view_random_blocks);
		emplace_block(conn_random_blocks);
	}

	auto fill_image = [&](auto& img, const auto& blocks, auto& offsets) {
		auto block_dim = props.get_block_dimensions(IMG_RESOLUTION);
		img.MakeRoom(props.dimensions / IMG_RESOLUTION);

		for (std::size_t i = 0; i < shuffled.size(); ++i) {
			auto offset = shuffled[i] * block_dim;
			copy_to_subimage(img, blocks[i], offset);
			offsets.push_back(offset);
		}
	};

	i3d::Image3d<T> view_img;
	std::vector<i3d::Vector3d<int>> view_offsets;
	i3d::Image3d<T> conn_img;
	std::vector<i3d::Vector3d<int>> conn_offsets;

	fill_image(view_img, view_random_blocks, view_offsets);
	fill_image(conn_img, conn_random_blocks, conn_offsets);

	phase_ok();

	phase_start("Write random blocks");

	{
		view.write_blocks(view_img, shuffled, view_offsets);
		assert(view.read_image<T>() == view_img);

		conn.write_blocks(conn_img, shuffled, conn_offsets, IMG_CHANNEL,
		                  IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
		                  IMG_VERSION);
		assert(view.read_image<T>() == conn_img);
	}

	phase_ok();

	phase_start("Read random blocks");

	{
		view.write_image(view_img);
		auto view_ret_blocks = view.read_blocks<T>(shuffled);
		assert(view_ret_blocks.size() == view_random_blocks.size());
		for (std::size_t i = 0; i < view_ret_blocks.size(); ++i)
			assert(view_ret_blocks[i] == view_random_blocks[i]);

		i3d::Image3d<T> view_cpy;
		view_cpy.MakeRoom(view_img.GetSize());
		view.read_blocks(shuffled, view_cpy, view_offsets);
		assert(view_cpy == view_img);

		view.write_image(conn_img);
		auto conn_ret_blocks =
		    conn.read_blocks<T>(shuffled, IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
		                        IMG_RESOLUTION, IMG_VERSION);

		assert(conn_ret_blocks.size() == conn_random_blocks.size());
		for (std::size_t i = 0; i < conn_ret_blocks.size(); ++i)
			assert(conn_ret_blocks[i] == conn_random_blocks[i]);

		i3d::Image3d<T> conn_cpy;
		conn_cpy.MakeRoom(conn_img.GetSize());
		conn.read_blocks(shuffled, conn_cpy, conn_offsets, IMG_CHANNEL,
		                 IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);
		assert(conn_cpy == conn_img);
	}

    phase_ok();
	test_ok();
}
} // namespace units