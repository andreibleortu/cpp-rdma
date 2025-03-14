# cpp-rdma

A high-performance RDMA (Remote Direct Memory Access) library providing both C and C++ interfaces for efficient network communications.

## Overview

`cpp-rdma` enables applications to leverage RDMA capabilities for high-throughput, low-latency communications by providing:

- Comprehensive C API for direct RDMA control
- Modern C++ wrappers with intuitive abstractions
- Support for all core RDMA operations (read, write, send/receive)
- Memory registration and management utilities
- Thread-safe resource handling

The library is built with performance in mind, allowing direct memory access between network-connected systems while bypassing CPU involvement in data transfers.

## Features

- **Multiple Communication Modes**:
  - Two-sided send/receive (traditional request-response pattern)
  - One-sided RDMA read (direct remote memory access)
  - One-sided RDMA write (direct remote memory writes)
  
- **C++ Abstractions**:
  - `RDMAConnection` - Connection management with RAII principles
  - `RDMAVector` - Remote array abstraction with transparent access
  - `RDMAVariable` - Remote variable with operator overloading

- **Clean Resource Management**:
  - Automatic cleanup with RAII in C++ interface
  - Signal handlers for graceful termination
  - Comprehensive error handling

## Architecture

The project is organized into distinct layers:

1. **Core Layer** - Low-level RDMA primitives and resource management
2. **Mode-Specific Layer** - Implementation of different RDMA operations
3. **C++ Wrapper Layer** - Object-oriented abstractions over the C API
4. **Examples** - Demonstration applications showing library usage

> **Note**: The library includes a Lambda mode (for remote function execution) which is a remnant of an older project. This functionality is largely experimental and not fully integrated into the current design.

## Project Structure

```
.
├── include/                # Public header files
│   ├── rdma/               # C API headers
│   │   ├── common.h        # Core RDMA definitions
│   │   ├── send_receive.h  # Two-sided communication
│   │   ├── rdma_read.h     # One-sided read operations
│   │   ├── rdma_write.h    # One-sided write operations
│   │   └── lambda.h        # Experimental remote execution
│   └── rdmacpp/           # C++ API headers
│       ├── rdma_connection.hpp   # Connection management
│       ├── rdma-vector.hpp       # Remote vector abstraction
│       └── rdma-variable.hpp     # Remote variable abstraction
├── src/                   # Implementation files
│   ├── core/              # Core C implementation
│   │   ├── common.c       # Common RDMA operations
│   │   └── rdma.c         # Main program entry point
│   ├── cpp/               # C++ wrappers
│   │   ├── rdma_connection.cpp
│   │   └── rdma-vector.cpp
│   ├── modes/             # RDMA operation modes
│   │   ├── lambda/        # Remote function execution (experimental)
│   │   ├── read/          # RDMA read operations
│   │   ├── send-recv/     # Two-sided communication
│   │   └── write/         # RDMA write operations
│   └── examples/          # Example applications
│       ├── rdma-example.cpp   # C++ API example
│       └── lambda-run.c       # Lambda mode example
├── bin/                   # Output directory for executables
├── lib/                   # Output directory for libraries
├── obj/                   # Output directory for object files
├── LICENSE                # GNU GPL v3 license
├── Makefile               # Build system
├── migrate.sh             # Project structure migration script
└── README.md              # This file
```

## Requirements

- Linux or Unix-based operating system
- RDMA-capable hardware (Mellanox/NVIDIA ConnectX, etc.)
- RDMA software stack (OFED or equivalent)
- GCC with C++11 support
- Libibverbs development package

## Building

```bash
# Install dependencies (Ubuntu/Debian example)
sudo apt-get install libibverbs-dev

# Build everything
make

# Build only the C library
make rdma

# Build C++ wrappers and examples
make rdmacpp

# Clean build artifacts
make clean
```

## Usage Examples

### C API Example

```c
#include "rdma/common.h"
#include "rdma/rdma_write.h"

int main() {
    // Server mode (for a write server)
    if (run_server(MODE_WRITE) != 0) {
        fprintf(stderr, "Failed to run server\n");
        return 1;
    }
    
    // Client mode (connecting to a write server)
    if (run_client("server_hostname", MODE_WRITE) != 0) {
        fprintf(stderr, "Failed to connect to server\n");
        return 1;
    }
    
    return 0;
}
```

### C++ API Example

```cpp
#include <iostream>
#include "rdmacpp/rdma_connection.hpp"
#include "rdmacpp/rdma-variable.hpp"

int main() {
    try {
        // Create a server connection
        RDMAConnection server(MODE_WRITE);
        
        // In client code:
        RDMAConnection client("server_hostname", MODE_WRITE);
        
        // Get remote memory region info
        qp_info_t remote_info = client.get_remote_mr_info();
        
        // Create an RDMA vector and variable
        RDMAVector<int> vec(&client, remote_info.addr, remote_info.rkey, 10);
        RDMAVariable<int> counter(&client, remote_info.addr + 40, remote_info.rkey);
        
        // Use them like regular C++ objects
        vec[3] = 42;
        counter += 10;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Library Modes

The library includes several RDMA operation modes:

### Two-Sided Send/Receive

Traditional RDMA communication requiring active participation from both sides. Good for request-response patterns with small messages.

### One-Sided RDMA Read

Allows direct reading from remote memory without involving the remote CPU. Excellent for data retrieval without disturbing the remote node.

### One-Sided RDMA Write

Enables direct writing to remote memory without involving the remote CPU. Perfect for updates and data distribution.

### Lambda Mode (Experimental)

Remnant from an older project, provides a framework for remote function execution. This mode is currently experimental and not fully integrated with the rest of the library.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.