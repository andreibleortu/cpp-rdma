// Generated from rdma-vector.cpp
#ifndef RDMA_VECTOR_HPP
#define RDMA_VECTOR_HPP

#include <iostream>
#include <cstdint>
#include <stdexcept>
#include "rdmacpp/rdma_connection.hpp"

class RDMAVector {
private:
    uint64_t remote_addr;    // Remote memory address
    uint32_t rkey;          // Remote memory key
    size_t size;            // Size of remote array
    RDMAConnection* conn;   // RDMA connection object

    // Proxy class for RDMA operations
    class RDMAReference {
    private:
        RDMAVector* parent;
        size_t index;
    
    public:
        RDMAReference(RDMAVector* p, size_t i) : parent(p), index(i) {}
        
        // Implicit conversion to read value
        operator int() const {
            int value = 0; // Initialize to avoid uninitialized value issues
            
            // Calculate the remote address for this element
            uint64_t elem_addr = parent->remote_addr + index * sizeof(int);
            
            // Use RDMA read to fetch the value from remote memory
            if (!parent->conn->rdma_read(&value, elem_addr, sizeof(int), parent->rkey)) {
                throw std::runtime_error("RDMA read operation failed");
            }
            return value;
        }
        
        // Assignment operator to write value
        RDMAReference& operator=(int value) {
            // Calculate the remote address for this element
            uint64_t elem_addr = parent->remote_addr + index * sizeof(int);
            
            // Use RDMA write to store the value to remote memory
            if (!parent->conn->rdma_write(&value, elem_addr, sizeof(int), parent->rkey)) {
                throw std::runtime_error("RDMA write operation failed");
            }
            return *this;
        }
    };

public:
    RDMAVector(RDMAConnection* connection, uint64_t remote_addr, uint32_t rkey, size_t size)
        : conn(connection), remote_addr(remote_addr), rkey(rkey), size(size) {
        if (!conn) {
            throw std::invalid_argument("Connection cannot be null");
        }
        if (size == 0) {
            throw std::invalid_argument("Vector size cannot be zero");
        }
    }
    
    // Overloaded [] operator returns the proxy object
    RDMAReference operator[](size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of bounds");
        }
        return RDMAReference(this, index);
    }
    
    // Get the size of the vector
    size_t get_size() const {
        return size;
    }
};

#endif // RDMA_VECTOR_HPP
