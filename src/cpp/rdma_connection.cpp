#include "rdmacpp/rdma_connection.hpp"
#include <cstring>

// Client constructor
RDMAConnection::RDMAConnection(const std::string& server_name, rdma_mode_t mode) 
    : owns_resources_(true) {
    
    // Initialize the config structure
    std::memset(&config_, 0, sizeof(config_t));
    std::memset(&remote_info_, 0, sizeof(qp_info_t));
    
    // Setup the connection
    rdma_status_t status = setup_rdma_connection(
        &config_, server_name.c_str(), mode, &remote_info_);
    
    if (status != RDMA_SUCCESS) {
        throw std::runtime_error("Failed to establish RDMA connection to server: " + 
                                server_name);
    }
}

// Server constructor
RDMAConnection::RDMAConnection(rdma_mode_t mode) 
    : owns_resources_(true) {
    
    // Initialize the config structure
    std::memset(&config_, 0, sizeof(config_t));
    std::memset(&remote_info_, 0, sizeof(qp_info_t));
    
    // Setup the connection
    rdma_status_t status = setup_rdma_connection(
        &config_, nullptr, mode, &remote_info_);
    
    if (status != RDMA_SUCCESS) {
        throw std::runtime_error("Failed to initialize RDMA server");
    }
}

RDMAConnection::~RDMAConnection() {
    if (owns_resources_) {
        cleanup_resources(&config_);
    }
}

bool RDMAConnection::rdma_read(void* local_buf, uint64_t remote_offset, size_t length) {
    // Use the negotiated remote memory region information
    return rdma_read(local_buf, remote_info_.addr + remote_offset, length, remote_info_.rkey);
}

bool RDMAConnection::rdma_read(void* local_buf, uint64_t remote_addr, size_t length, uint32_t rkey) {
    if (!local_buf || length == 0 || length > MAX_BUFFER_SIZE) {
        return false;
    }
    
    // Create a temporary qp_info for the remote memory
    struct qp_info_t temp_remote_info = {0};
    temp_remote_info.addr = remote_addr;
    temp_remote_info.rkey = rkey;
    
    // Need to copy data from our buffer if it's different
    bool using_direct_buf = (local_buf == config_.buf);
    
    // Post the read operation (reading into config_.buf)
    post_operation(&config_, OP_READ, nullptr, &temp_remote_info, length);
    
    // Wait for completion
    wait_completion(&config_);
    
    // Copy data to the user's buffer if needed
    if (!using_direct_buf) {
        std::memcpy(local_buf, config_.buf, length);
    }
    
    return true;
}

bool RDMAConnection::rdma_write(const void* local_buf, uint64_t remote_offset, size_t length) {
    // Use the negotiated remote memory region information
    return rdma_write(local_buf, remote_info_.addr + remote_offset, length, remote_info_.rkey);
}

bool RDMAConnection::rdma_write(const void* local_buf, uint64_t remote_addr, size_t length, uint32_t rkey) {
    if (!local_buf || length == 0 || length > MAX_BUFFER_SIZE) {
        return false;
    }
    
    // Create a temporary qp_info for the remote memory
    struct qp_info_t temp_remote_info = {0};
    temp_remote_info.addr = remote_addr;
    temp_remote_info.rkey = rkey;
    
    // Copy data to our registered buffer if needed
    if (local_buf != config_.buf) {
        std::memcpy(config_.buf, local_buf, length);
    }
    
    // Post the write operation
    post_operation(&config_, OP_WRITE, static_cast<const char*>(config_.buf), &temp_remote_info, length);
    
    // Wait for completion
    wait_completion(&config_);
    
    return true;
}

qp_info_t RDMAConnection::get_local_mr_info(void* addr) const {
    qp_info_t info;
    info.qp_num = config_.qp->qp_num;
    info.gid = config_.gid;
    info.addr = addr ? reinterpret_cast<uint64_t>(addr) : reinterpret_cast<uint64_t>(config_.buf);
    info.rkey = config_.mr->rkey;
    return info;
}

qp_info_t RDMAConnection::get_remote_mr_info() const {
    return remote_info_;
}

ibv_mr* RDMAConnection::register_memory(void* addr, size_t length, int access_flags) {
    if (!addr || length == 0) {
        return nullptr;
    }
    
    return ibv_reg_mr(config_.pd, addr, length, access_flags);
}
