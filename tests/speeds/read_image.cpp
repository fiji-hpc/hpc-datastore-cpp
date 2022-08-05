#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <chrono>
#include <i3d/image3d.h>

template <typename T>
void meassure() {
	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);	

	std::cout << "Starting meassurement\n";
	auto start = std::chrono::steady_clock::now();

	i3d::Image3d<T> img = img_view.read_image<T>();

	auto end = std::chrono::steady_clock::now();

	double secs = double(std::chrono::duration_cast<std::chrono::milliseconds>(
	                         end - start)
	                         .count()) /
	              1000.0;
	std::size_t bytes = img.GetImageSize() * sizeof(T);
	std::cout << "Reading took: " << secs << " seconds\n";
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