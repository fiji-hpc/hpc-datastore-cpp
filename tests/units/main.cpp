#include "../../src/hpc_ds_api.hpp"
#include "block.hpp"
#include "blocks.hpp"
#include "image.hpp"
#include "region.hpp"

int main() {
	auto props = ds::get_dataset_properties(SERVER_IP, SERVER_PORT, DS_UUID);

	SELECT_TYPE(props->voxel_type, units::test_block)
	SELECT_TYPE(props->voxel_type, units::test_blocks)
	SELECT_TYPE(props->voxel_type, units::test_region)
	SELECT_TYPE(props->voxel_type, units::test_image)
};