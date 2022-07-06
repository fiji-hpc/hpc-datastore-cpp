#pragma once

#include "../common.hpp"
#include <i3d/transform.h>
#include <iostream>

namespace units {
template <typename T>
void test_image() {
	test_start("Read/Write Image");

	phase_start("Fetch dataset properties");
	auto props = ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);
	phase_ok();

	phase_start("Generate random images");
	i3d::Image3d<T> random_img, view_random_img, conn_random_img;

	auto img_dim = props.get_img_dimensions(IMG_RESOLUTION);
	random_img.MakeRoom(img_dim);
	view_random_img.MakeRoom(img_dim);
	conn_random_img.MakeRoom(img_dim);

	fill_random(random_img);
	fill_random(view_random_img);
	fill_random(conn_random_img);

	phase_ok();

	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);
	ds::ImageView view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                   IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	/*
	phase_start("Read/Write image");
	{
	    ds::write_image(random_img, SERVER_IP, SERVER_PORT, DS_UUID,
	                    IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                    IMG_VERSION);
	    auto got = ds::read_image<T>(SERVER_IP, SERVER_PORT, DS_UUID,
	                                 IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
	                                 IMG_RESOLUTION, IMG_VERSION);

	    assert(random_img == got);
	}
	phase_ok();

	phase_start("Read/Write image using Connection");

	{
	    conn.write_image(conn_random_img, IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
	                     IMG_RESOLUTION, IMG_VERSION);
	    auto conn_got = conn.read_image<T>(
	        IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);
	    assert(conn_random_img == conn_got);
	}

	phase_ok();

	phase_start("Read/Write image using ImageView");

	{
	    view.write_image(view_random_img);
	    auto view_got = view.read_image<T>();
	    assert(view_random_img == view_got);
	}

	phase_ok();
	*/
	phase_start("Write with pyramids");

	{
		std::vector<ds::SamplingMode> modes = {
		    // ds::SamplingMode::LANCZOS, ds::SamplingMode::LINEAR, <--- not
		    // suported by i3d
		    ds::SamplingMode::NEAREST_NEIGHBOUR};

		for (auto& mode : modes) {
			fill_random(random_img);
			ds::write_with_pyramids(random_img, SERVER_IP, SERVER_PORT, DS_UUID,
			                        IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
			                        IMG_VERSION, mode);

			for (auto& resolution : props.get_all_resolutions()) {
				i3d::Image3d<T> cpy;
				i3d::Resample(random_img, cpy,
				              props.get_img_dimensions(resolution), mode);

				assert(cpy == conn.read_image<T>(IMG_CHANNEL, IMG_TIMEPOINT,
				                                 IMG_ANGLE, resolution,
				                                 IMG_VERSION));
			}
		}
	}

	phase_ok();

	phase_start("Write with pyramids using Connection");

	{

		std::vector<ds::SamplingMode> modes = {
		    // ds::SamplingMode::LANCZOS, ds::SamplingMode::LINEAR, <--- not
		    // suported by i3d
		    ds::SamplingMode::NEAREST_NEIGHBOUR};

		for (auto& mode : modes) {
			fill_random(random_img);
			conn.write_with_pyramids(random_img, IMG_CHANNEL, IMG_TIMEPOINT,
			                         IMG_ANGLE, IMG_VERSION, mode);

			for (auto& resolution : props.get_all_resolutions()) {
				i3d::Image3d<T> cpy;
				i3d::Resample(random_img, cpy,
				              props.get_img_dimensions(resolution), mode);

				assert(cpy == conn.read_image<T>(IMG_CHANNEL, IMG_TIMEPOINT,
				                                 IMG_ANGLE, resolution,
				                                 IMG_VERSION));
			}
		}
	}

	phase_ok();

	test_ok();
}
} // namespace units