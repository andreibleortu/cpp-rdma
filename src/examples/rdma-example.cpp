#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <csignal>
#include "rdmacpp/rdma_connection.hpp"
#include "rdmacpp/rdma-variable.hpp"

// Include the RDMAVector class
#include "rdmacpp/rdma-vector.hpp"

// Global flag for graceful shutdown
volatile bool running = true;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    std::cout << "Caught signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

// Example server function
void server_function() {
    try {
        std::cout << "Starting RDMA server..." << std::endl;
        
        // Set up signal handling
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        
        // Create a server connection using read/write mode
        RDMAConnection server(MODE_RW);
        
        // Allocate and initialize a buffer for an integer array
        const size_t array_size = 10;
        int* int_array = new int[array_size];
        for (size_t i = 0; i < array_size; i++) {
            int_array[i] = static_cast<int>(i * 10);
        }
        
        // Register this memory for RDMA access
        ibv_mr* array_mr = server.register_memory(
            int_array, array_size * sizeof(int), 
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
        
        if (!array_mr) {
            throw std::runtime_error("Failed to register memory region");
        }
        
        // Allocate a single counter variable
        int* counter = new int(42);
        
        // Register this memory for RDMA access
        ibv_mr* counter_mr = server.register_memory(
            counter, sizeof(int),
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
        
        if (!counter_mr) {
            ibv_dereg_mr(array_mr);
            delete[] int_array;
            delete counter;
            throw std::runtime_error("Failed to register memory region for counter");
        }
        
        // Get QP information obtained during connection setup
        qp_info_t remote_info = server.get_remote_mr_info();
        
        std::cout << "Server ready. Array and counter registered for RDMA access." << std::endl;
        std::cout << "Memory regions: " << std::endl;
        std::cout << "  Array address: " << (void*)int_array << ", rkey: 0x" << std::hex << array_mr->rkey << std::dec << std::endl;
        std::cout << "  Counter address: " << (void*)counter << ", rkey: 0x" << std::hex << counter_mr->rkey << std::dec << std::endl;
        std::cout << "  Remote memory base address: 0x" << std::hex << remote_info.addr << std::dec << std::endl;
        
        // Wait for operations to complete or shutdown signal
        std::cout << "Server waiting. Press Ctrl+C to exit..." << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Current counter value: " << *counter << std::endl;
        }
        
        // Cleanup
        std::cout << "Cleaning up resources..." << std::endl;
        ibv_dereg_mr(array_mr);
        ibv_dereg_mr(counter_mr);
        delete[] int_array;
        delete counter;
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

// Example client function
void client_function(const std::string& server_name) {
    try {
        std::cout << "Connecting to RDMA server: " << server_name << std::endl;
        
        // Create a client connection
        RDMAConnection client(server_name, MODE_RW);
        
        // Get remote info obtained during connection setup
        qp_info_t remote_info = client.get_remote_mr_info();
        
        std::cout << "Connected to server. Remote memory base address: 0x" << std::hex 
                  << remote_info.addr << ", rkey: 0x" << remote_info.rkey << std::dec << std::endl;
        
        // Create RDMA abstractions for remote memory
        // Use negotiated remote address and rkey from connection setup
        RDMAVector array(&client, remote_info.addr, remote_info.rkey, 10);
        RDMAVariable<int> counter(&client, remote_info.addr + 40, remote_info.rkey); // 40 = 10 ints * 4 bytes
        
        // Read and display the initial values
        std::cout << "Initial values from server:" << std::endl;
        std::cout << "  Counter = " << static_cast<int>(counter) << std::endl;
        std::cout << "  Array contents: ";
        for (size_t i = 0; i < array.get_size(); i++) {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
        
        // Modify values using the C++ abstractions
        std::cout << "Modifying values..." << std::endl;
        
        // Increment counter multiple times
        std::cout << "  Incrementing counter by 1" << std::endl;
        counter++;
        std::cout << "  Counter is now: " << static_cast<int>(counter) << std::endl;
        
        std::cout << "  Incrementing counter by 10" << std::endl;
        counter += 10;
        std::cout << "  Counter is now: " << static_cast<int>(counter) << std::endl;
        
        // Modify array elements
        std::cout << "  Setting array[3] = 999" << std::endl;
        array[3] = 999;
        
        std::cout << "  Setting array[5] = array[2] * 2" << std::endl;
        array[5] = array[2] * 2;
        
        // Read and display the updated values
        std::cout << "Updated values from server:" << std::endl;
        std::cout << "  Counter = " << static_cast<int>(counter) << std::endl;
        std::cout << "  Array contents: ";
        for (size_t i = 0; i < array.get_size(); i++) {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

// Simple command-line argument parser
void print_usage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Server mode: ./rdma-example server" << std::endl;
    std::cout << "  Client mode: ./rdma-example client <server>" << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  ./rdma-example client localhost" << std::endl;
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string mode(argv[1]);
    
    if (mode == "server") {
        server_function();
    } 
    else if (mode == "client" && argc >= 3) {
        std::string server_name(argv[2]);
        client_function(server_name);
    } 
    else {
        print_usage();
        return 1;
    }
    
    return 0;
}
