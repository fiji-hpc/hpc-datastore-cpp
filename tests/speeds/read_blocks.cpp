#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <chrono>
#include <i3d/image3d.h>

template <typename T>
void meassure() {
	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);

	std::cout << "Searching for blocks to request\n";
	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	i3d::Vector3d<int> img_dim = props.dimensions / IMG_RESOLUTION;
	i3d::Vector3d<int> block_dim = props.get_block_dimensions(IMG_RESOLUTION);
	i3d::Vector3d<int> block_count =
	    (img_dim + block_dim - 1) / block_dim; // Ceiling
	std::vector<i3d::Vector3d<int>> blocks;

	for (int x = 0; x < block_count.x; ++x)
		for (int y = 0; y < block_count.y; ++y)
			for (int z = 0; z < block_count.z; ++z)
				blocks.emplace_back(x, y, z);

	shuffle(blocks);

	std::cout << "Starting meassurement\n";
	auto start = std::chrono::steady_clock::now();

	for (auto block : blocks)
		auto img = img_view.read_block<T>(block); 

	auto end = std::chrono::steady_clock::now();

	double secs = double(std::chrono::duration_cast<std::chrono::milliseconds>(
	                         end - start)
	                         .count()) /
	              1000.0;
	std::size_t bytes = img_dim.x * img_dim.y * img_dim.z * sizeof(T);
	std::cout << "Reading took: " << secs << " seconds\n";
	std::cout << "Downloaded: " << bytes << " bytes\n";
	std::cout << "Average speed: " << double(bytes) / 1'000'000.0 / secs
	          << " MB/s\n";
}

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;

	/** Select correct format for template **/
	SELECT_TYPE(props.voxel_type, meassure);
}