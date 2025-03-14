#ifndef RDMA_VARIABLE_HPP
#define RDMA_VARIABLE_HPP

#include <iostream>
#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include "rdmacpp/rdma_connection.hpp"

/**
 * @brief Single variable abstraction for RDMA operations
 *
 * Provides transparent access to a remote variable using RDMA operations.
 * Uses operator overloading and implicit conversions for natural syntax.
 */
template<typename T>
class RDMAVariable {
private:
    uint64_t remote_addr;    // Remote memory address
    uint32_t rkey;           // Remote memory key
    RDMAConnection* conn;    // RDMA connection object
    
public:
    // Constructor for RDMA-ifying a variable
    RDMAVariable(RDMAConnection* connection, uint64_t remote_addr, uint32_t rkey)
        : conn(connection), remote_addr(remote_addr), rkey(rkey) {
        if (!conn) {
            throw std::invalid_argument("Connection cannot be null");
        }
    }
    
    // Implicit conversion operator - automatically called when variable is read
    operator T() const {
        T value{};  // Zero-initialize to avoid undefined behavior
        // Use RDMA read to fetch the value from remote memory
        if (!conn->rdma_read(&value, remote_addr, sizeof(T), rkey)) {
            throw std::runtime_error("RDMA read operation failed");
        }
        return value;
    }
    
    // Assignment operator - called when variable is written to
    RDMAVariable& operator=(const T& value) {
        // Use RDMA write to store the value to remote memory
        if (!conn->rdma_write(&value, remote_addr, sizeof(T), rkey)) {
            throw std::runtime_error("RDMA write operation failed");
        }
        return *this;
    }
    
    // Compound assignment operators
    RDMAVariable& operator+=(const T& value) {
        T current = *this; // Calls the conversion operator to get current value
        current += value;
        *this = current;   // Calls assignment operator to store result
        return *this;
    }
    
    RDMAVariable& operator-=(const T& value) {
        T current = *this;
        current -= value;
        *this = current;
        return *this;
    }
    
    RDMAVariable& operator*=(const T& value) {
        T current = *this;
        current *= value;
        *this = current;
        return *this;
    }
    
    RDMAVariable& operator/=(const T& value) {
        if (value == 0) {
            throw std::domain_error("Division by zero");
        }
        T current = *this;
        current /= value;
        *this = current;
        return *this;
    }
    
    // Pre-increment
    RDMAVariable& operator++() {
        T value = *this;
        ++value;
        *this = value;
        return *this;
    }
    
    // Post-increment
    T operator++(int) {
        T old_value = *this;
        T new_value = old_value + 1;
        *this = new_value;
        return old_value;
    }
    
    // Pre-decrement
    RDMAVariable& operator--() {
        T value = *this;
        --value;
        *this = value;
        return *this;
    }
    
    // Post-decrement
    T operator--(int) {
        T old_value = *this;
        T new_value = old_value - 1;
        *this = new_value;
        return old_value;
    }
};

#endif // RDMA_VARIABLE_HPP