#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <chrono>
#include <i3d/image3d.h>


template <typename T>
void meassure() {
	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);
    std::cout << "Generating random image\n";
    auto props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);
    i3d::Vector3d<int> img_dim = props->get_img_dimensions(IMG_RESOLUTION);
	i3d::Image3d<T> new_img;
	new_img.MakeRoom(img_dim);
	fill_random(new_img);

	std::cout << "Starting meassurement\n";
	auto start = std::chrono::steady_clock::now();

	img_view.write_image(new_img);

	auto end = std::chrono::steady_clock::now();

	double secs = double(std::chrono::duration_cast<std::chrono::milliseconds>(
	                         end - start)
	                         .count()) /
	              1000.0;
	std::size_t bytes = new_img.GetImageSize() * sizeof(T);
	std::cout << "Writing took: " << secs << " seconds\n";
	std::cout << "Downloaded: " << bytes << " bytes\n";
	std::cout << "Average speed: " << double(bytes) / 1'000'000.0 / secs
	          << " MB/s\n";
}

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	auto props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;

	/** Select correct format for template **/
	SELECT_TYPE(props->voxel_type, meassure);
}