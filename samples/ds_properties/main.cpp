#include "../../src/hpc_ds_api.hpp"
#include "../common.hpp"
#include <iostream>

int main() {
	std::cout << "Fetching properties from the server ... " << std::flush;

	ds::DatasetProperties props =
	    ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	std::cout << "[OK]" << std::endl;
	std::cout << "Properties:\n\n" << props << '\n';
}

