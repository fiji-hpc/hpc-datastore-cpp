#pragma once

#include "../common.hpp"
#include <iostream>

namespace units {
template <typename T>
void test_region() {
	test_start("Read/Write regions");

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

	phase_start("Generating random blocks");

	std::vector<i3d::Vector3d<int>> shuffled = avail_coords;
	shuffle(shuffled);
	std::vector<i3d::Image3d<T>> random_blocks;

	for (auto coord : shuffled) {
		auto emplace_block = [&](auto& vec) {
			vec.emplace_back();
			auto& img = vec.back();

			img.MakeRoom(props.get_block_size(coord, IMG_RESOLUTION));
			fill_random(img);
		};

		emplace_block(random_blocks);
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

	i3d::Image3d<T> random_img;
	std::vector<i3d::Vector3d<int>> img_offsets;

	fill_image(random_img, random_blocks, img_offsets);

	phase_ok();

	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);
	ds::ImageView view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                   IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	phase_start("Region matching image");

	view.write_image(random_img);
	// random_img.SaveImage("random.tif");
	{
		i3d::Image3d<T> conn_got = conn.read_region<T>(
		    {0, 0, 0}, random_img.GetSize(), IMG_CHANNEL, IMG_TIMEPOINT,
		    IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

		// conn_got.SaveImage("conn.tif");
		assert(conn_got == random_img);

		conn_got.SetAllVoxels(0);
		conn.read_region({0, 0, 0}, random_img.GetSize(), conn_got, {0, 0, 0},
		                 IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
		                 IMG_VERSION);
		assert(conn_got == random_img);
	}

	{
		i3d::Image3d<T> view_got =
		    view.read_region<T>({0, 0, 0}, random_img.GetSize());
		assert(view_got == random_img);

		view_got.SetAllVoxels(0);
		view.read_region({0, 0, 0}, random_img.GetSize(), view_got, {0, 0, 0});
		assert(view_got == random_img);
	}

	phase_ok();

	phase_start("Region matching block");
	{
		auto block_dim = props.get_block_dimensions(IMG_RESOLUTION);
		i3d::Image3d<T> view_img;
		i3d::Image3d<T> conn_img;

		view_img.MakeRoom(random_img.GetSize());
		conn_img.MakeRoom(random_img.GetSize());

		for (std::size_t i = 0; i < shuffled.size(); ++i) {
			auto& coord = shuffled[i];
			auto& block = random_blocks[i];
			auto start_point = block_dim * coord;
			auto end_point =
			    block_dim * coord + props.get_block_size(coord, IMG_RESOLUTION);

			std::cout << "Got here" << std::endl;
			auto view_got = view.read_region<T>(start_point, end_point);
			assert(view_got == block);

			std::cout << "And here" << std::endl;
			view.read_region(start_point, end_point, view_img, start_point);

			std::cout << "And here too" << std::endl;
			auto conn_got = conn.read_region<T>(
			    start_point, end_point, IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
			    IMG_RESOLUTION, IMG_VERSION);
			assert(conn_got == block);

			std::cout << "And here as well" << std::endl;
			conn.read_region(start_point, end_point, conn_img, start_point,
			                 IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
			                 IMG_RESOLUTION, IMG_VERSION);
		}
		assert(view_img == random_img);
		assert(conn_img == random_img);
	}
	phase_ok();

	phase_ok();

	test_ok();
}
} // namespace units