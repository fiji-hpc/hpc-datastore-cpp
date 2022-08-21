#pragma once
#include "hpc_ds_details.hpp"
#include "hpc_ds_structs.hpp"
#include <fmt/core.h>
#include <i3d/image3d.h>
#include <i3d/transform.h>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ds {
/**
 * @brief Get the dataset properties object
 *
 * Creates a HTTP requests to server <ip>:<port> and collects data about dataset
 * <uuid>. If some property is not found, atribute is left at its default value
 * (Warning is emmited if in DEBUG).
 *
 * @param ip IP address of server (http:// at the beginning is not necessary)
 * @param port Port, where the server is listening for requests
 * @param uuid Unique identifier of dataset
 * @return DatasetProperties
 */
inline dataset_props_ptr get_dataset_properties(const std::string& ip,
                                                int port,
                                                const std::string& uuid);

/**
 * @brief Read full image
 *
 * Creates (several if needed) HTTP requests and collects whole image and
 * returns it. All image properties are collected from server side.
 *
 * As there is no (meaningfull) way for C++ to choose correct underlying type in
 * runtime, make sure to specify correct template type.
 *
 * @tparam T Scalar used as underlying type for image representation
 * @param ip IP address of server (http:// at the beginning is not necessary)
 * @param port Port, where the server is listening for requests
 * @param uuid Unique identifier of dataset
 * @param channel Channel, at which the image is located
 * @param timepoint Timepoint, at which the image is located
 * @param angle Angle, at which the image is located
 * @param resolution Resolution, at which the image is located
 * @param version Version, at which the image is located (integer identifier or
 * "latest")
 * @param props [Optional] cached dataset properties
 * @return i3d::Image3d<T>
 */
template <cnpts::Scalar T>
i3d::Image3d<T> read_image(const std::string& ip,
                           int port,
                           const std::string& uuid,
                           int channel = 0,
                           int timepoint = 0,
                           int angle = 0,
                           i3d::Vector3d<int> resolution = {1, 1, 1},
                           const std::string& version = "latest",
                           dataset_props_ptr props = nullptr);

/**
 * @brief Write full image
 *
 * Creates (several if needed) HTTP requests and sends whole image to datastore.
 *
 * @tparam T Scalar used as underlying type for image representation
 * @param img Input image to be send to server
 * @param ip IP address of server (http:// at the beginning is not necessary)
 * @param port Port, where the server is listening for requests
 * @param uuid Unique identifier of dataset
 * @param channel Channel, at which the image is located
 * @param timepoint Timepoint, at which the image is located
 * @param angle Angle, at which the image is located
 * @param resolution Resolution, at which the image is located
 * @param version Version, at which the image is located (integer identifier or
 * "latest")
 * @param props [Optional] cached dataset properties
 */
template <cnpts::Scalar T>
void write_image(const i3d::Image3d<T>& img,
                 const std::string& ip,
                 int port,
                 const std::string& uuid,
                 int channel = 0,
                 int timepoint = 0,
                 int angle = 0,
                 i3d::Vector3d<int> resolution = {1, 1, 1},
                 const std::string version = "latest",
                 dataset_props_ptr props = nullptr);

/**
 * @brief Write full image and generate pyramids
 *
 * Creates (several if needed) HTTP requests and sends whole image to datastore.
 * Input image is considered to be full-resolution (that is: {1, 1, 1}).
 * All other resolutions will be generated with selected <ResamplingMode>
 * and uploaded to server as well.
 *
 * @tparam T Scalar used as underlying type for image representation
 * @param img Input image in original resolution
 * @param ip IP address of server (http:// at the beginning is not necessary)
 * @param port Port, where the server is listening for requests
 * @param uuid Unique identifier of dataset
 * @param channel Channel, at which the image is located
 * @param timepoint Timepoint, at which the image is located
 * @param angle Angle, at which the image is located
 * @param version Version, at which the image is located (integer identifier or
 * "latest")
 * @param m Sampling mode used for image resampling
 * @param props [Optional] cached dataset properties
 */
template <cnpts::Scalar T>
void write_with_pyramids(const i3d::Image3d<T>& img,
                         const std::string& ip,
                         int port,
                         const std::string& uuid,
                         int channel = 0,
                         int timepoint = 0,
                         int angle = 0,
                         const std::string& version = "latest",
                         SamplingMode m = SamplingMode::NEAREST_NEIGHBOUR,
                         dataset_props_ptr props = nullptr);

/**
 * @brief Representation of connection to specific image
 *
 * Class representing connection to specific image on the server.
 * This class provides basic methods for read/write operations necessary to
 * transfer images from/to server. This class does not cache or precollect
 * any data, so the first HTTP request will be send only when corresponding
 * function is called.
 */
class ImageView {
  public:
	/**
	 * @brief Construct a new Image View object
	 *
	 * @param ip IP address of server (http:// at the beginning is not
	 * necessary)
	 * @param port Port, where the server is listening for requests
	 * @param uuid Unique identifier of dataset
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	ImageView(std::string ip,
	          int port,
	          std::string uuid,
	          int channel,
	          int timepoint,
	          int angle,
	          i3d::Vector3d<int> resolution,
	          std::string version);

	/**
	 * @brief Get dataset properties
	 *
	 * @return DatasetProperties
	 */
	dataset_props_ptr get_properties() const;

	/**
	 * @brief Read one block from server
	 *
	 * Reads one block of image located at <coord> and returns it.
	 * The information about size of the image is collected from the server.
	 *
	 * If in DEBUG, function will check wheter given coordinate corresponds
	 * to valid block.
	 *
	 * As there is no (meaningfull) way for C++ to choose correct underlying
	 * type in runtime, make sure to specify correct template type.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coord Block coordinate
	 * @param props [Optional] cached dataset properties
	 * @return Image containing selected block
	 */
	template <cnpts::Scalar T>
	i3d::Image3d<T> read_block(i3d::Vector3d<int> coord,
	                           dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read one block from server to image
	 *
	 * Reads one block of image located at <coord> and saves it to <dest> with
	 * offset <dest_offset>.
	 *
	 * If in DEBUG, function will check wheter given coordinate corresponds
	 * to valid block as well as wheter the block fits into the image (taking
	 * offset into account).
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coord Block coordinate
	 * @param dest Image to write data to
	 * @param dest_offset Offset by which the corresponding write should be
	 * moved
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void read_block(i3d::Vector3d<int> coord,
	                i3d::Image3d<T>& dest,
	                i3d::Vector3d<int> dest_offset = {0, 0, 0},
	                dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read blocks from server and return them
	 *
	 * Reads blocks specified in <coords> and returns them. Corresponding
	 * sizes are collected from server and calculated specificaly for each
	 * block.
	 *
	 * This function is not optimized, meaning that for each coord in <coord>,
	 * one HTTP request will be sent out to the server. This can heavily slow
	 * down speed of the application as communication via network is not cheap.
	 * If you do not have specific needs,most of the time it will be faster
	 * to collect blocks into prealocated image (second overload of
	 * read_blocks), however it will eat more RAM.
	 *
	 * If in DEBUG, the fucntion checks if coordinates given in <coords> points
	 * to a valid blocks.
	 *
	 * As there is no (meaningfull) way for C++ to choose correct underlying
	 * type in runtime, make sure to specify correct template type.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coords Block coordinates
	 * @param props [Optional] cached dataset properties
	 * @return Vector of fetched blocks (the order is the same as given in
	 * <coords>)
	 */
	template <cnpts::Scalar T>
	std::vector<i3d::Image3d<T>>
	read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
	            dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read blocks from server and saves them into prealocated image
	 *
	 * Read blocks specified in <coords> and saves them into locations given in
	 * <offsets>.
	 *
	 * If in DEBUG, the function checks if coordinates given in <coords> points
	 * to a valid blocks.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coords Block coordinates
	 * @param dest Prealocated destination image
	 * @param offsets Offsets at wich the corresponding blocks should be saved
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
	                 i3d::Image3d<T>& dest,
	                 const std::vector<i3d::Vector3d<int>>& offsets,
	                 dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read region of interest from the server
	 *
	 * Read all neccessary blocks intersecting with chosen region from the
	 * server. It is neccessary, that start_point < end_point (elem-wise)..
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param start_point smallest point of the region
	 * @param end_point largest point of the region
	 * @param props [Optional] cached dataset properties
	 * @return i3d::Image3d<T> Selected region
	 */
	template <cnpts::Scalar T>
	i3d::Image3d<T> read_region(i3d::Vector3d<int> start_point,
	                            i3d::Vector3d<int> end_point,
	                            dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read region of interest from the server
	 *
	 * Read all neccessary blocks intersecting with chosen region from the
	 * server and insert region into preallocated image <dest> at <offset>.
	 *
	 * It is neccessary, that start_point < end_point (elem-wise)..
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param start_point smallest point of the region
	 * @param end_point largest point of the region
	 * @param dest destination image
	 * @param offset offset to destination image
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void read_region(i3d::Vector3d<int> start_point,
	                 i3d::Vector3d<int> end_point,
	                 i3d::Image3d<T>& dest,
	                 i3d::Vector3d<int> offset = {0, 0, 0},
	                 dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read full image
	 *
	 * Read full image from the server and return it. The information about
	 * dimensions are fetched from the server.
	 *
	 * As there is no (meaningfull) way for C++ to choose correct underlying
	 * type in runtime, make sure to specify correct template type.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param props [Optional] cached dataset properties
	 * @return i3d::Image3d<T> Fetched image
	 */
	template <cnpts::Scalar T>
	i3d::Image3d<T> read_image(dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Write block to server
	 *
	 * Write block from source image to server. The information about
	 * dimensions are fetched from the server.
	 *
	 * If in DEBUG, the function checks if coordinate given in <coord> points to
	 * a valid block, as well as wheter the offset specified for block is within
	 * image boundaries.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param src Source image to collect block from
	 * @param coord Block coordinates
	 * @param src_offset Offset of given block in source image
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_block(const i3d::Image3d<T>& src,
	                 i3d::Vector3d<int> coord,
	                 i3d::Vector3d<int> src_offset = {0, 0, 0},
	                 dataset_props_ptr props = nullptr) const;
	/**
	 * @brief Write blocks to server
	 *
	 * Write blocks from source image to server. The information about
	 * dimensions are fetched from the server.
	 *
	 * If in DEBUG, the function checks if coordinates given in <coords> points
	 * to a valid block, as well as wheter the offsets specified for each block
	 * is within image boundaries.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param src Source image to collect blocks from
	 * @param coords Vector of block coordinates
	 * @param src_offsets Offsets of corresponding blocks in source image
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_blocks(const i3d::Image3d<T>& src,
	                  const std::vector<i3d::Vector3d<int>>& coords,
	                  const std::vector<i3d::Vector3d<int>>& src_offsets,
	                  dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Write image to server
	 *
	 * Write full image to server.
	 *
	 * It is recommended to make sure that the dimension of the source image is
	 * the same as the dimension of image at server side.
	 *
	 * Mostly, given smaller source image will emit error and fail to upload.
	 * Given larger source image will result in cropping.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param img Source image
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_image(const i3d::Image3d<T>& img,
	                 dataset_props_ptr props = nullptr) const;

  private:
	std::string _ip;
	int _port;
	std::string _uuid;
	int _channel;
	int _timepoint;
	int _angle;
	i3d::Vector3d<int> _resolution;
	std::string _version;
};

/**
 * @brief Representation of connection to dataset
 *
 * Class representing connection to specific dataset on the server.
 * It provides basic methods for read/write operations necessary to tranfser
 * images (in the dataset) from/to server. This class does not cache or
 * precollect any data, so the first HTTP request will be send only when
 * corresponding function is called.
 *
 * All of the methods accepts arguments that uniquely identifies requested
 * image. At the backend, this class tranfsers commands into ImageView objects.
 */
class Connection {
  public:
	/**
	 * @brief Construct a new Connection object
	 *
	 * @param ip IP address of server (http:// at the beginning is not
	 * necessary)
	 * @param port Port, where the server is listening for requests
	 * @param uuid Unique identifier of dataset
	 */
	Connection(std::string ip, int port, std::string uuid);

	/**
	 * @brief Get ImageView of specified image.
	 *
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @return ImageView
	 */
	ImageView get_view(int channel,
	                   int timepoint,
	                   int angle,
	                   i3d::Vector3d<int> resolution,
	                   const std::string& version) const;

	/**
	 * @brief Get dataset properties
	 *
	 * @return DatasetProperties
	 */
	dataset_props_ptr get_properties() const;

	/**
	 * @brief Read one block from server to image
	 *
	 * Reads one block of image located at <coord> and saves it to <dest> with
	 * offset <dest_offset>.
	 *
	 * If in DEBUG, function will check wheter given coordinate corresponds
	 * to valid block as well as wheter the block fits into the image (taking
	 * offset into account).
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coord Block coordinate
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 * @return Image containing selected block
	 */
	template <cnpts::Scalar T>
	i3d::Image3d<T> read_block(i3d::Vector3d<int> coord,
	                           int channel,
	                           int timepoint,
	                           int angle,
	                           i3d::Vector3d<int> resolution,
	                           const std::string& version,
	                           dataset_props_ptr props = nullptr) const;
	/**
	 * @brief Read one block from server to image
	 *
	 * Reads one block of image located at <coord> and saves it to <dest> with
	 * offset <dest_offset>.
	 *
	 * If in DEBUG, function will check wheter given coordinate corresponds
	 * to valid block as well as wheter the block fits into the image (taking
	 * offset into account).
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coord Block coordinate
	 * @param dest Image to write data to
	 * @param dest_offset Offset by which the corresponding write should be
	 * moved
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void read_block(i3d::Vector3d<int> coord,
	                i3d::Image3d<T>& dest,
	                i3d::Vector3d<int> dest_offset,
	                int channel,
	                int timepoint,
	                int angle,
	                i3d::Vector3d<int> resolution,
	                const std::string& version,
	                dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read blocks from server and return them
	 *
	 * Reads blocks specified in <coords> and returns them. Corresponding
	 * sizes are collected from server and calculated specificaly for each
	 * block.
	 *
	 * This function is not optimized, meaning that for each coord in <coord>,
	 * one HTTP request will be sent out to the server. This can heavily slow
	 * down speed of the application as communication via network is not cheap.
	 * If you do not have specific needs,most of the time it will be faster
	 * to collect blocks into prealocated image (second overload of
	 * read_blocks), however it will eat more RAM.
	 *
	 * If in DEBUG, the fucntion checks if coordinates given in <coords> points
	 * to a valid blocks.
	 *
	 * As there is no (meaningfull) way for C++ to choose correct underlying
	 * type in runtime, make sure to specify correct template type.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coords Block coordinates
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 * @return Vector of fetched blocks (the order is the same as given in
	 * <coords>)
	 */
	template <cnpts::Scalar T>
	std::vector<i3d::Image3d<T>>
	read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
	            int channel,
	            int timepoint,
	            int angle,
	            i3d::Vector3d<int> resolution,
	            const std::string& version,
	            dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read blocks from server and saves them into prealocated image
	 *
	 * Read blocks specified in <coords> and saves them into locations given in
	 * <offsets>.
	 *
	 * If in DEBUG, the function checks if coordinates given in <coords> points
	 * to a valid blocks, as well as wheter the offsets specified for each block
	 * are within image boundaries.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param coords Block coordinates
	 * @param dest Prealocated destination image
	 * @param dest_offsets Offsets at wich the corresponding blocks should be
	 * saved
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
	                 i3d::Image3d<T>& dest,
	                 const std::vector<i3d::Vector3d<int>>& dest_offsets,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 i3d::Vector3d<int> resolution,
	                 const std::string& version,
	                 dataset_props_ptr props = nullptr) const;
	/**
	 * @brief Read region of interest from the server
	 *
	 * Read all neccessary blocks intersecting with chosen region from the
	 * server. It is neccessary, that start_point < end_point (elem-wise)..
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param start_point smallest point of the region
	 * @param end_point largest point of the region
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 * @return Image containing selected block
	 */
	template <cnpts::Scalar T>
	i3d::Image3d<T> read_region(i3d::Vector3d<int> start_point,
	                            i3d::Vector3d<int> end_point,
	                            int channel,
	                            int timepoint,
	                            int angle,
	                            i3d::Vector3d<int> resolution,
	                            const std::string& version,
	                            dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read region of interest from the server
	 *
	 * Read all neccessary blocks intersecting with chosen region from the
	 * server and insert region into preallocated image <dest> at <offset>.
	 *
	 * It is neccessary, that start_point < end_point (elem-wise)..
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param start_point smallest point of the region
	 * @param end_point largest point of the region
	 * @param dest destination image
	 * @param offset offset to destination image
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void read_region(i3d::Vector3d<int> start_point,
	                 i3d::Vector3d<int> end_point,
	                 i3d::Image3d<T>& dest,
	                 i3d::Vector3d<int> offset,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 i3d::Vector3d<int> resolution,
	                 const std::string& version,
	                 dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Read full image
	 *
	 * Read full image from the server and return it. The information about
	 * dimensions are fetched from the server.
	 *
	 * As there is no (meaningfull) way for C++ to choose correct underlying
	 * type in runtime, make sure to specify correct template type.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 * @return i3d::Image3d<T> etched image
	 */
	template <cnpts::Scalar T>
	i3d::Image3d<T> read_image(int channel,
	                           int timepoint,
	                           int angle,
	                           i3d::Vector3d<int> resolution,
	                           const std::string& version,
	                           dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Write block to server
	 *
	 * Write block from source image to server. The information about
	 * dimensions are fetched from the server.
	 *
	 * If in DEBUG, the function checks if coordinate given in <coord> points to
	 * a valid block, as well as wheter the offset specified for block is within
	 * image boundaries.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param src Source image to collect block from
	 * @param coord Block coordinates
	 * @param src_offset Offset of given block in source image
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_block(const i3d::Image3d<T>& src,
	                 i3d::Vector3d<int> coord,
	                 i3d::Vector3d<int> src_offset,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 i3d::Vector3d<int> resolution,
	                 const std::string& version,
	                 dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Write blocks to server
	 *
	 * Write blocks from source image to server. The information about
	 * dimensions are fetched from the server.
	 *
	 * If in DEBUG, the function checks if coordinates given in <coords> points
	 * to a valid block, as well as wheter the offsets specified for each block
	 * is within image boundaries.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param src Source image to collect blocks from
	 * @param coords Vector of block coordinates
	 * @param src_offsets Offsets of corresponding blocks in source image
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_blocks(const i3d::Image3d<T>& src,
	                  const std::vector<i3d::Vector3d<int>>& coords,
	                  const std::vector<i3d::Vector3d<int>>& src_offsets,
	                  int channel,
	                  int timepoint,
	                  int angle,
	                  i3d::Vector3d<int> resolution,
	                  const std::string& version,
	                  dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Write image to server
	 *
	 * Write full image to server.
	 *
	 * It is recommended to make sure that the dimension of the source image is
	 * the same as the dimension of image at server side.
	 *
	 * Mostly, given smaller source image will emit error and fail to upload.
	 * Given larger source image will result in cropping.
	 *
	 * @tparam T Scalar used as underlying type for image representation
	 * @param img Source image
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param resolution Resolution, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
	 * or "latest")
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_image(const i3d::Image3d<T>& img,
	                 int channel,
	                 int timepoint,
	                 int angle,
	                 i3d::Vector3d<int> resolution,
	                 const std::string& version,
	                 dataset_props_ptr props = nullptr) const;

	/**
	 * @brief Write full image and generate pyramids
	 *
	 * Creates (several if needed) HTTP requests and sends whole image to
	datastore.
	    * Input image is considered to be full-resolution (that is: {1, 1, 1}).
	* All other resolutions will be generated with selected <ResamplingMode>
	* and uploaded to server as well.

	 * @tparam T Scalar used as underlying type for image representation
	 * @param img Input image in original resolution
	 * @param channel Channel, at which the image is located
	 * @param timepoint Timepoint, at which the image is located
	 * @param angle Angle, at which the image is located
	 * @param version Version, at which the image is located (integer identifier
 or
 * "latest")
	 * @param m Sampling mode used for image resampling
	 * @param props [Optional] cached dataset properties
	 */
	template <cnpts::Scalar T>
	void write_with_pyramids(const i3d::Image3d<T>& img,
	                         int channel,
	                         int timepoint,
	                         int angle,
	                         const std::string& version,
	                         SamplingMode m,
	                         dataset_props_ptr props = nullptr) const;

  private:
	std::string _ip;
	int _port;
	std::string _uuid;
};

} // namespace ds

/* ================= IMPLEMENTATION FOLLOWS ======================== */

namespace ds {
/* ===================================== Global space */
/* inline */ dataset_props_ptr get_dataset_properties(const std::string& ip,
                                                      int port,
                                                      const std::string& uuid) {
	std::string dataset_url = details::get_dataset_url(ip, port, uuid);
	return std::make_unique<DatasetProperties>(
	    details::get_dataset_properties(dataset_url));
}

template <cnpts::Scalar T>
i3d::Image3d<T> read_image(const std::string& ip,
                           int port,
                           const std::string& uuid,
                           int channel /* = 0 */,
                           int timepoint /*  = 0 */,
                           int angle /* = 0 */,
                           i3d::Vector3d<int> resolution /* = {1, 1, 1} */,
                           const std::string& version /* = "latest" */,
                           dataset_props_ptr props /* = nullptr */) {
	return ImageView(ip, port, uuid, channel, timepoint, angle, resolution,
	                 version)
	    .read_image<T>(props);
}

template <cnpts::Scalar T>
void write_image(const i3d::Image3d<T>& img,
                 const std::string& ip,
                 int port,
                 const std::string& uuid,
                 int channel /* = 0 */,
                 int timepoint /* = 0 */,
                 int angle /* = 0 */,
                 i3d::Vector3d<int> resolution /*  = {1, 1, 1} */,
                 const std::string version /* = "latest" */,
                 dataset_props_ptr props /* = nullptr */) {
	ImageView(ip, port, uuid, channel, timepoint, angle, resolution, version)
	    .write_image(img, props);
}

template <cnpts::Scalar T>
void write_with_pyramids(const i3d::Image3d<T>& img,
                         const std::string& ip,
                         int port,
                         const std::string& uuid,
                         int channel /* = 0 */,
                         int timepoint /* = 0 */,
                         int angle /* = 0 */,
                         const std::string& version /* = "latest"*/,
                         SamplingMode m /* = SamplingMode::NEAREST_NEIGHBOUR */,
                         dataset_props_ptr props /* = nullptr */) {
	Connection(ip, port, uuid)
	    .write_with_pyramids(img, channel, timepoint, angle, version, m, props);
}

/* ===================================== ImageView */

ImageView::ImageView(std::string ip,
                     int port,
                     std::string uuid,
                     int channel,
                     int timepoint,
                     int angle,
                     i3d::Vector3d<int> resolution,
                     std::string version)
    : _ip(std::move(ip)), _port(port), _uuid(std::move(uuid)),
      _channel(channel), _timepoint(timepoint), _angle(angle),
      _resolution(resolution), _version(std::move(version)) {}

dataset_props_ptr ImageView::get_properties() const {
	return get_dataset_properties(_ip, _port, _uuid);
}

template <cnpts::Scalar T>
i3d::Image3d<T>
ImageView::read_block(i3d::Vector3d<int> coord,
                      dataset_props_ptr props /* = nullptr */) const {
	/* Fetch properties from server */
	if (!props)
		props = get_properties();

	i3d::Vector3d<int> block_dim = props->get_block_dimensions(_resolution);

	/* Prepare output image */
	i3d::Image3d<T> img;
	i3d::Vector3d<int> block_size = details::data_manip::get_block_size(
	    coord, block_dim, props->get_img_dimensions(_resolution));
	img.MakeRoom(block_size);

	/* Fetch and return */
	read_block(coord, img);
	return img;
}

template <cnpts::Scalar T>
void ImageView::read_block(i3d::Vector3d<int> coord,
                           i3d::Image3d<T>& dest,
                           i3d::Vector3d<int> dest_offset /*  = {0, 0, 0} */,
                           dataset_props_ptr props /* = nullptr */) const {
	read_blocks({coord}, dest, {dest_offset}, props);
}

template <cnpts::Scalar T>
std::vector<i3d::Image3d<T>>
ImageView::read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
                       dataset_props_ptr props /* = nullptr */) const {
	if (!props)
		props = get_properties();

	/* Process blocks one by one */
	std::vector<i3d::Image3d<T>> out;
	for (auto coord : coords)
		out.push_back(read_block<T>(coord));

	return out;
}

template <cnpts::Scalar T>
void ImageView::read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
                            i3d::Image3d<T>& dest,
                            const std::vector<i3d::Vector3d<int>>& offsets,
                            dataset_props_ptr props /* = nullptr */
) const {

	if (!props)
		props = get_properties();

	if (!details::matches_image_type(dest, props->voxel_type))
		throw std::logic_error("Server and i3d image type does not match\n");

	auto resolutions = props->get_all_resolutions();
	if (std::ranges::find(resolutions, _resolution) == end(resolutions))
		throw std::logic_error(
		    fmt::format("Resolution {} not supported by server\n",
		                details::to_string(_resolution))
		        .c_str());

	if (!props->timepoint_ids.contains(_timepoint))
		throw std::logic_error(
		    fmt::format("Timepoint {} not supported by server\n",
		                details::to_string(_timepoint))
		        .c_str());

	if (_channel >= props->channels)
		throw std::logic_error(
		    fmt::format("Channel {} not supported by server\n",
		                details::to_string(_channel))
		        .c_str());

	if (_angle >= props->angles)
		throw std::logic_error(fmt::format("Angle {} not supported by server\n",
		                                   details::to_string(_angle))
		                           .c_str());

	/* Fetched properties from server */
	std::string dataset_url = details::get_dataset_url(_ip, _port, _uuid);
	i3d::Vector3d<int> block_dim = props->get_block_dimensions(_resolution);

	i3d::Vector3d<int> img_dim = props->get_img_dimensions(_resolution);

	if (coords.size() != offsets.size())
		throw std::logic_error("Count of coordinates != count of offsets");

	if (!details::check_block_coords(coords, img_dim, block_dim))
		throw std::out_of_range("Blocks out of range");

	/* prepare request url */
	std::string session_url = details::requests::session_url_request(
	    dataset_url, _resolution, _version);

	if (session_url.ends_with('/'))
		session_url.pop_back();

	std::vector<std::pair<std::string, std::vector<std::size_t>>> requests =
	    details::create_requests(coords, session_url, _timepoint, _channel,
	                             _angle);

	for (const auto& [req, idxs] : requests) {
		auto [data, response] = details::requests::make_request(req);

		std::size_t start_i = 0;
		for (std::size_t i : idxs) {
			i3d::Vector3d<int> block_size =
			    props->get_block_size(coords[i], _resolution);
			std::size_t data_size = details::data_manip::get_block_data_size(
			    block_size, props->voxel_type);

			details::data_manip::read_data(
			    std::span(data.begin() + start_i, data_size), props->voxel_type,
			    dest, offsets[i], block_size);

			start_i += data_size;
		}
	}
}

template <cnpts::Scalar T>
i3d::Image3d<T>
ImageView::read_region(i3d::Vector3d<int> start_point,
                       i3d::Vector3d<int> end_point,
                       dataset_props_ptr props /* = nullptr */) const {
	if (!props)
		props = get_properties();

	i3d::Vector3d img_dim = props->get_img_dimensions(_resolution);
	i3d::Vector3d block_dim = props->get_block_dimensions(_resolution);

	std::vector<i3d::Vector3d<int>> coords = details::get_intercepted_blocks(
	    start_point, end_point, img_dim, block_dim);

	std::vector<i3d::Vector3d<int>> offsets;
	for (auto coord : coords)
		offsets.emplace_back(coord * block_dim - start_point);

	i3d::Image3d<T> out_img;
	out_img.MakeRoom(end_point - start_point);

	read_blocks(coords, out_img, offsets, props);
	return out_img;
}

template <cnpts::Scalar T>
void ImageView::read_region(i3d::Vector3d<int> start_point,
                            i3d::Vector3d<int> end_point,
                            i3d::Image3d<T>& dest,
                            i3d::Vector3d<int> offset /* = {0, 0, 0} */,
                            dataset_props_ptr props /* = nullptr */) const {
	auto temp_img = read_region<T>(start_point, end_point, props);

	// Copy to desired location
	for (std::size_t x = 0; x < temp_img.GetSizeX(); ++x)
		for (std::size_t y = 0; y < temp_img.GetSizeY(); ++y)
			for (std::size_t z = 0; z < temp_img.GetSizeZ(); ++z)
				dest.SetVoxel(x + offset.x, y + offset.y, z + offset.z,
				              temp_img.GetVoxel({x, y, z}));
}

template <cnpts::Scalar T>
i3d::Image3d<T>
ImageView::read_image(dataset_props_ptr props /* = nullptr */) const {
	if (!props)
		props = get_properties();

	i3d::Vector3d img_dim = props->get_img_dimensions(_resolution);
	return read_region<T>(0, img_dim, props);
}

template <cnpts::Scalar T>
void ImageView::write_block(const i3d::Image3d<T>& src,
                            i3d::Vector3d<int> coord,
                            i3d::Vector3d<int> src_offset /*  = {0, 0, 0} */,
                            dataset_props_ptr props /* = nullptr */) const {
	write_blocks(src, {coord}, {src_offset}, props);
}

template <cnpts::Scalar T>
void ImageView::write_blocks(const i3d::Image3d<T>& src,
                             const std::vector<i3d::Vector3d<int>>& coords,
                             const std::vector<i3d::Vector3d<int>>& src_offsets,
                             dataset_props_ptr props /* = nullptr */) const {

	if (!props)
		props = get_properties();

	if (!details::matches_image_type(src, props->voxel_type))
		throw std::logic_error("Server and i3d image type does not match\n");

	auto resolutions = props->get_all_resolutions();
	if (std::ranges::find(resolutions, _resolution) == end(resolutions))
		throw std::logic_error(
		    fmt::format("Resolution {} not supported by server\n",
		                details::to_string(_resolution))
		        .c_str());

	if (!props->timepoint_ids.contains(_timepoint))
		throw std::logic_error(
		    fmt::format("Timepoint {} not supported by server\n",
		                details::to_string(_timepoint))
		        .c_str());

	if (_channel >= props->channels)
		throw std::logic_error(
		    fmt::format("Channel {} not supported by server\n",
		                details::to_string(_channel))
		        .c_str());

	if (_angle >= props->angles)
		throw std::logic_error(fmt::format("Angle {} not supported by server\n",
		                                   details::to_string(_angle))
		                           .c_str());

	/* Fetch server properties */
	std::string dataset_url = details::get_dataset_url(_ip, _port, _uuid);

	i3d::Vector3d<int> block_dim = props->get_block_dimensions(_resolution);
	i3d::Vector3d<int> img_dim = props->get_img_dimensions(_resolution);

	/* Error checking (when not in debug, all checks automatically return
	 * true)*/
	if (coords.size() != src_offsets.size())
		throw std::logic_error("Count of coordinates != count of offsets");

	if (!details::check_block_coords(coords, img_dim, block_dim))
		throw std::out_of_range("Blocks out of range");

	/* prepare request url */
	std::string session_url = details::requests::session_url_request(
	    dataset_url, _resolution, _version);

	if (session_url.ends_with('/'))
		session_url.pop_back();

	std::vector<std::pair<std::string, std::vector<std::size_t>>> requests =
	    details::create_requests(
	        coords, session_url, _timepoint, _channel, _angle,
	        134217728 /
	            (sizeof(T) * block_dim.x * block_dim.y *
	             block_dim.z)); //<-- Magic constant empiricaly chosen :D

	for (const auto& [req, idxs] : requests) {
		std::size_t full_size = 0;
		for (std::size_t i : idxs)
			full_size += details::data_manip::get_block_data_size(
			    props->get_block_size(coords[i], _resolution),
			    props->voxel_type);

		/* Prepare vector representing octet-data (will be send to server) */
		std::vector<char> data(full_size);

		/* Transform image to octet-data */
		std::size_t start_i = 0;
		for (std::size_t i : idxs) {
			i3d::Vector3d<int> block_size =
			    props->get_block_size(coords[i], _resolution);
			std::size_t data_size = details::data_manip::get_block_data_size(
			    block_size, props->voxel_type);

			details::data_manip::write_data(
			    src, src_offsets[i],
			    std::span(data.begin() + start_i, data_size), props->voxel_type,
			    block_size);

			start_i += data_size;
		}

		auto [_, response] = details::requests::make_request(
		    req, Poco::Net::HTTPRequest::HTTP_POST, data,
		    {{"Content-Type", "application/octet-stream"}});
	}
}

template <cnpts::Scalar T>
void ImageView::write_image(const i3d::Image3d<T>& img,
                            dataset_props_ptr props /* = nullptr */) const {

	/* Fetch image properties from server */
	if (!props)
		props = get_properties();

	auto resolutions = props->get_all_resolutions();
	if (std::ranges::find(resolutions, _resolution) == end(resolutions) ||
	    !eq(props->get_img_dimensions(_resolution), img.GetSize()))
		throw std::logic_error("Size of server and i3d image does not match\n");

	i3d::Vector3d<int> block_dim = props->get_block_dimensions(_resolution);
	i3d::Vector3d<int> img_dim = props->get_img_dimensions(_resolution);
	i3d::Vector3d<int> block_count =
	    (img_dim + block_dim - 1) / block_dim; // Ceiling

	/* Prepare coordinates of blocks and offsets to write whole image */
	std::vector<i3d::Vector3d<int>> blocks;
	std::vector<i3d::Vector3d<int>> offsets;

	for (int x = 0; x < block_count.x; ++x)
		for (int y = 0; y < block_count.y; ++y)
			for (int z = 0; z < block_count.z; ++z) {
				blocks.emplace_back(x, y, z);
				offsets.emplace_back(x * block_dim.x, y * block_dim.y,
				                     z * block_dim.z);
			}

	/* write whole image */
	write_blocks(img, blocks, offsets, props);
}

/* ===================================== Connection */

Connection::Connection(std::string ip, int port, std::string uuid)
    : _ip(std::move(ip)), _port(port), _uuid(std::move(uuid)) {}

ImageView Connection::get_view(int channel,
                               int timepoint,
                               int angle,
                               i3d::Vector3d<int> resolution,
                               const std::string& version) const {
	return ImageView(_ip, _port, _uuid, channel, timepoint, angle, resolution,
	                 version);
}

dataset_props_ptr Connection::get_properties() const {
	return get_dataset_properties(_ip, _port, _uuid);
}

template <cnpts::Scalar T>
i3d::Image3d<T>
Connection::read_block(i3d::Vector3d<int> coord,
                       int channel,
                       int timepoint,
                       int angle,
                       i3d::Vector3d<int> resolution,
                       const std::string& version,
                       dataset_props_ptr props /* = nullptr */) const {
	return get_view(channel, timepoint, angle, resolution, version)
	    .read_block<T>(coord, props);
}

template <cnpts::Scalar T>
void Connection::read_block(i3d::Vector3d<int> coord,
                            i3d::Image3d<T>& dest,
                            i3d::Vector3d<int> dest_offset,
                            int channel,
                            int timepoint,
                            int angle,
                            i3d::Vector3d<int> resolution,
                            const std::string& version,
                            dataset_props_ptr props /* = nullptr */) const {
	return get_view(channel, timepoint, angle, resolution, version)
	    .read_block(coord, dest, dest_offset, props);
}

template <cnpts::Scalar T>
std::vector<i3d::Image3d<T>>
Connection::read_blocks(const std::vector<i3d::Vector3d<int>>& coords,
                        int channel,
                        int timepoint,
                        int angle,
                        i3d::Vector3d<int> resolution,
                        const std::string& version,
                        dataset_props_ptr props /* = nullptr */) const {
	return get_view(channel, timepoint, angle, resolution, version)
	    .read_blocks<T>(coords, props);
}

template <cnpts::Scalar T>
void Connection::read_blocks(
    const std::vector<i3d::Vector3d<int>>& coords,
    i3d::Image3d<T>& dest,
    const std::vector<i3d::Vector3d<int>>& dest_offsets,
    int channel,
    int timepoint,
    int angle,
    i3d::Vector3d<int> resolution,
    const std::string& version,
    dataset_props_ptr props /* = nullptr */) const {
	get_view(channel, timepoint, angle, resolution, version)
	    .read_blocks(coords, dest, dest_offsets, props);
}

template <cnpts::Scalar T>
i3d::Image3d<T>
Connection::read_region(i3d::Vector3d<int> start_point,
                        i3d::Vector3d<int> end_point,
                        int channel,
                        int timepoint,
                        int angle,
                        i3d::Vector3d<int> resolution,
                        const std::string& version,
                        dataset_props_ptr props /* = nullptr */) const {
	return get_view(channel, timepoint, angle, resolution, version)
	    .read_region<T>(start_point, end_point, props);
}

template <cnpts::Scalar T>
void Connection::read_region(i3d::Vector3d<int> start_point,
                             i3d::Vector3d<int> end_point,
                             i3d::Image3d<T>& dest,
                             i3d::Vector3d<int> offset,
                             int channel,
                             int timepoint,
                             int angle,
                             i3d::Vector3d<int> resolution,
                             const std::string& version,
                             dataset_props_ptr props /* = nullptr */) const {
	get_view(channel, timepoint, angle, resolution, version)
	    .read_region<T>(start_point, end_point, dest, offset, props);
}

template <cnpts::Scalar T>
i3d::Image3d<T>
Connection::read_image(int channel,
                       int timepoint,
                       int angle,
                       i3d::Vector3d<int> resolution,
                       const std::string& version,
                       dataset_props_ptr props /* = nullptr */) const {
	return get_view(channel, timepoint, angle, resolution, version)
	    .read_image<T>(props);
}

template <cnpts::Scalar T>
void Connection::write_block(const i3d::Image3d<T>& src,
                             i3d::Vector3d<int> coord,
                             i3d::Vector3d<int> src_offset,
                             int channel,
                             int timepoint,
                             int angle,
                             i3d::Vector3d<int> resolution,
                             const std::string& version,
                             dataset_props_ptr props /* = nullptr */) const {
	get_view(channel, timepoint, angle, resolution, version)
	    .write_block(src, coord, src_offset, props);
}

template <cnpts::Scalar T>
void Connection::write_blocks(
    const i3d::Image3d<T>& src,
    const std::vector<i3d::Vector3d<int>>& coords,
    const std::vector<i3d::Vector3d<int>>& src_offsets,
    int channel,
    int timepoint,
    int angle,
    i3d::Vector3d<int> resolution,
    const std::string& version,
    dataset_props_ptr props /* = nullptr */) const {
	get_view(channel, timepoint, angle, resolution, version)
	    .write_blocks(src, coords, src_offsets, props);
}

template <cnpts::Scalar T>
void Connection::write_image(const i3d::Image3d<T>& img,
                             int channel,
                             int timepoint,
                             int angle,
                             i3d::Vector3d<int> resolution,
                             const std::string& version,
                             dataset_props_ptr props /* = nullptr */) const {
	get_view(channel, timepoint, angle, resolution, version)
	    .write_image(img, props);
}

template <cnpts::Scalar T>
void Connection::write_with_pyramids(
    const i3d::Image3d<T>& img,
    int channel,
    int timepoint,
    int angle,
    const std::string& version,
    SamplingMode m,
    dataset_props_ptr props /* = nullptr */) const {
	if (!props)
		props = get_properties();
	write_image(img, channel, timepoint, angle, {1, 1, 1}, version, props);

	for (const auto& res : props->get_all_resolutions()) {
		if (res == i3d::Vector3d<int>{1, 1, 1})
			continue;

		i3d::Vector3d<int> new_dim = props->get_img_dimensions(res);
		i3d::Image3d<T> cpy;
		i3d::Resample(img, cpy, new_dim, m);
		write_image(cpy, channel, timepoint, angle, res, version, props);
	}
}

} // namespace ds
