#include "../../src/hpc_ds_api.hpp"
#include "read_block.hpp"

int main() {
	auto props = ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	SELECT_TYPE(props.voxel_type, units::test_block)
};