#ifndef RDMA_CONNECTION_HPP
#define RDMA_CONNECTION_HPP

#include <memory>
#include <stdexcept>
#include <string>

// Use extern "C" for C header includes
extern "C" {
#include "rdma/common.h"
}

/**
 * @brief C++ wrapper class for RDMA connections
 *
 * Provides a high-level C++ interface to the underlying C RDMA functionality,
 * with proper resource management via RAII principles.
 */
class RDMAConnection
{
private:
	struct config_t config_;
	struct qp_info_t remote_info_; // Store remote QP info from negotiation
	bool owns_resources_;

public:
	/**
	 * @brief Construct a client RDMA connection
	 * @param server_name Server hostname or IP address
	 * @param mode RDMA operation mode
	 */
	RDMAConnection(const std::string &server_name, rdma_mode_t mode);

	/**
	 * @brief Construct a server RDMA connection
	 * @param mode RDMA operation mode
	 */
	RDMAConnection(rdma_mode_t mode);

	/**
	 * @brief Destructor ensures proper cleanup of RDMA resources
	 */
	~RDMAConnection();

	// Disable copying
	RDMAConnection(const RDMAConnection &) = delete;
	RDMAConnection &operator=(const RDMAConnection &) = delete;

	/**
	 * @brief Perform RDMA read operation using negotiated remote info
	 * @param local_buf Local buffer to store read data
	 * @param remote_offset Offset from remote buffer base address
	 * @param length Number of bytes to read
	 * @return True if operation succeeds
	 */
	bool rdma_read(void *local_buf, uint64_t remote_offset, size_t length);

	/**
	 * @brief Perform RDMA read operation with explicit remote address and key
	 * @param local_buf Local buffer to store read data
	 * @param remote_addr Remote memory address to read from
	 * @param length Number of bytes to read
	 * @param rkey Remote memory key
	 * @return True if operation succeeds
	 */
	bool rdma_read(void *local_buf, uint64_t remote_addr, size_t length, uint32_t rkey);

	/**
	 * @brief Perform RDMA write operation using negotiated remote info
	 * @param local_buf Local buffer containing data to write
	 * @param remote_offset Offset from remote buffer base address
	 * @param length Number of bytes to write
	 * @return True if operation succeeds
	 */
	bool rdma_write(const void *local_buf, uint64_t remote_offset, size_t length);

	/**
	 * @brief Perform RDMA write operation with explicit remote address and key
	 * @param local_buf Local buffer containing data to write
	 * @param remote_addr Remote memory address to write to
	 * @param length Number of bytes to write
	 * @param rkey Remote memory key
	 * @return True if operation succeeds
	 */
	bool rdma_write(const void *local_buf, uint64_t remote_addr, size_t length, uint32_t rkey);

	/**
	 * @brief Get local memory region info
	 * @param addr Address to store in returned info
	 * @return QP info for the local memory region
	 */
	qp_info_t get_local_mr_info(void *addr = nullptr) const;

	/**
	 * @brief Get remote memory region info obtained during connection
	 * @return Remote QP info from negotiation
	 */
	qp_info_t get_remote_mr_info() const;

	/**
	 * @brief Register a local memory region for RDMA operations
	 * @param addr Memory address to register
	 * @param length Size of the memory region
	 * @param access_flags Access permissions
	 * @return Registered memory region
	 */
	ibv_mr *register_memory(void *addr, size_t length, int access_flags);
};

#endif // RDMA_CONNECTION_HPP
