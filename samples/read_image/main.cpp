#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <iostream>

namespace ds = datastore;

template <typename T>
void get_image() {
	// Fetching image
	std::cout << "Fetching image ... " << std::flush;
	i3d::Image3d<T> image =
	    ds::read_image<T>(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                   IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);
	std::cout << "[OK]" << std::endl;

	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);

	// Fetch image using connection
	std::cout << "Fetching image using Connection ... " << std::flush;
	i3d::Image3d<T> connection_image = conn.read_image<T>(
	    IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	std::cout << "[OK]" << std::endl;

	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);

	// Fetch image using image view
	std::cout << "Fetching image using ImageView ... " << std::flush;
	i3d::Image3d<T> view_image = img_view.read_image<T>();
	std::cout << "[OK]" << std::endl;

	std::cout << "Checking if all fetched images are the same ... " << std::flush;
	std::cout << std::boolalpha
	          << (image == connection_image && image == view_image)
	          << std::endl;

	std::cout << "Saving to image.tif\n";
	image.SaveImage("image.tif");
}

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;

	/** Select correct format for template **/
	SELECT_TYPE(props.voxel_type, get_image);
}
