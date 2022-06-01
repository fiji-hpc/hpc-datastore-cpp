#include "hpc_ds_api.hpp"
#include <iostream>
#include <string>

const std::string ip = "http://127.0.0.1";
constexpr unsigned short port = 9080;
const std::string uuid = "64aec83b-332e-45a6-9aed-0051d7bb2cf8";

int main() {
	std::cout << std::string(datastore::get_dataset_properties(ip, port, uuid))
	          << '\n';
}

