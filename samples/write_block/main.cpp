#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <iostream>

template <typename T>
void store_block() {
	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);
	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);

	// Fetching block to get size
	std::cout << "Fetching block {0,0,0} to get block size ... " << std::flush;
	i3d::Vector3d<int> block_dim = img_view.read_block<T>({0, 0, 0}).GetSize();
	std::cout << "[OK]" << std::endl;

	std::cout << "Creating random block ... " << std::flush;
	// create block
	i3d::Image3d<T> new_block;
	new_block.MakeRoom(block_dim);
	fill_random(new_block);
	std::cout << "[OK]" << std::endl;

	// Upload image using connection
	std::cout << "Uploading block using Connection ... " << std::flush;
	conn.write_block(new_block, {0, 0, 0}, {0, 0, 0}, IMG_CHANNEL,
	                 IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	std::cout << "[OK]" << std::endl;

	// Uploading block using image view
	std::cout << "Uploading block using ImageView ... " << std::flush;
	img_view.write_block(new_block, {0, 0, 0}, {0, 0, 0});
	std::cout << "[OK]" << std::endl;

	// Fetch block from server
	std::cout << "Fetching block from server for confirmation ... "
	          << std::flush;
	i3d::Image3d<T> server_block = img_view.read_block<T>({0, 0, 0});
	std::cout << "[OK]" << std::endl;

	std::cout << "Checking if blocks are the same ... " << std::flush;
	std::cout << std::boolalpha << (new_block == server_block) << std::endl;

	/**
	 * NOTE: It is also possible to upload more block at once using
	 * xxx.write_blocks(...) method, see documentation for more information. **/

	std::cout << "Saving to block.tif\n";
	server_block.SaveImage("block.tif");
}

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	ds::dataset_props_ptr props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;

	/** Select correct format for template **/
	SELECT_TYPE(props->voxel_type, store_block);
}
