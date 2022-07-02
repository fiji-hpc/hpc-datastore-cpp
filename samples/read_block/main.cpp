#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <iostream>

template <typename T>
void get_block() {
	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);

	// Fetch new block
	std::cout << "Fetching new block {0, 0, 0} using Connection ... "
	          << std::flush;
	i3d::Image3d<T> connection_block =
	    conn.read_block<T>({0, 0, 0}, IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
	                       IMG_RESOLUTION, IMG_VERSION);

	std::cout << "[OK]" << std::endl;
	/* It is also possible to collect muliple blocks
	 * conn.read_blocks<T>({{0,0,0}, {1, 0, 0}}, IMG_CHANNEL, ... );
	 */

	// Fetch block to already existing image
	std::cout << "Fetching block {0, 0, 0} in to existing image using "
	             "Connection ... "
	          << std::flush;
	i3d::Image3d<T> connection_block_inplace;

	/* Allocate space */
	connection_block_inplace.MakeRoom(connection_block.GetSize());

	conn.read_block({0, 0, 0}, connection_block_inplace, {0, 0, 0}, IMG_CHANNEL,
	                IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	std::cout << "[OK]" << std::endl;

	/* Same can be done with multiple blocks using same principle as above (see
	 * documentation for more info) */

	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);

	// Fetch new block
	std::cout << "Fetching new block {0, 0, 0} using ImageView ... "
	          << std::flush;
	i3d::Image3d<T> view_block = img_view.read_block<T>({0, 0, 0});
	std::cout << "[OK]" << std::endl;

	/* Same principle as above for collecting multiple blocks...
	img_view.read_blocks({{0,0,0}, {1, 0, 0}, ... });
	*/

	// Fetch block to already existing image
	std::cout
	    << "Fetching block {0, 0, 0} in to existing image using ImageView ... "
	    << std::flush;
	i3d::Image3d<T> view_block_inplace;

	/* Allocate space */
	view_block_inplace.MakeRoom(view_block.GetSize());

	img_view.read_block({0, 0, 0}, view_block_inplace, {0, 0, 0});
	
	std::cout << "[OK]" << std::endl;

	std::cout << "Checking if all recovered images are the same ... "
	          << std::flush;
	std::cout << std::boolalpha
	          << (connection_block == connection_block_inplace &&
	              connection_block == view_block &&
	              connection_block == view_block_inplace)
	          << std::endl;

	std::cout << "Saving to block.tif\n";
	connection_block.SaveImage("block.tif");
}

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;

	/** Select correct format for template **/
	SELECT_TYPE(props.voxel_type, get_block);
}
