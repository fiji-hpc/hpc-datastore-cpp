#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <iostream>

namespace ds = datastore;

template <typename T>
void store_image() {
	// Fetching image
	std::cout
	    << "Fetching properties from the server to obtain basic dimensions ... "
	    << std::flush;
	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);
	std::cout << "[OK]" << std::endl;

	std::cout << "Creating random image ... " << std::flush;
	// create image
	i3d::Vector3d<int> img_dim = props.dimensions / IMG_RESOLUTION;
	i3d::Image3d<T> new_img;
	new_img.MakeRoom(img_dim);
	fill_random(new_img);
	std::cout << "[OK]" << std::endl;

	// Upload image
	std::cout << "Uploading image ... " << std::flush;
	ds::write_image(new_img, SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION, IMG_VERSION);

	std::cout << "[OK]" << std::endl;

	ds::Connection conn(SERVER_IP, SERVER_PORT, DS_UUID);

	// Upload image using connection
	std::cout << "Uploading image using Connection ... " << std::flush;
	conn.write_image(new_img, IMG_CHANNEL, IMG_TIMEPOINT, IMG_ANGLE,
	                 IMG_RESOLUTION, IMG_VERSION);

	std::cout << "[OK]" << std::endl;

	ds::ImageView img_view(SERVER_IP, SERVER_PORT, DS_UUID, IMG_CHANNEL,
	                       IMG_TIMEPOINT, IMG_ANGLE, IMG_RESOLUTION,
	                       IMG_VERSION);

	// Uploading image using image view
	std::cout << "Uploading image using ImageView ... " << std::flush;
	img_view.write_image(new_img);
	std::cout << "[OK]" << std::endl;

	// Fetch image from server
	std::cout << "Fetching image from server for confirmation ... "
	          << std::flush;
	i3d::Image3d<T> server_img = img_view.read_image<T>();
	std::cout << "[OK]" << std::endl;

	std::cout << "Checking if images are the same ... " << std::flush;
	std::cout << std::boolalpha << (new_img == server_img) << std::endl;

	std::cout << "Saving to image.tif\n";
	server_img.SaveImage("image.tif");
}

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;

	/** Select correct format for template **/
	SELECT_TYPE(props.voxel_type, store_image);
}
