# cpp-rdma

Student showcase project exploring RDMA with a small C core and a "RDMA-ified" C++ layer that aims to feel like normal C++ usage via operator overloading.

## Intent

This repo is meant to:

- Demonstrate core RDMA concepts (QP setup, MR registration, read/write)
- Provide a minimal C API for low-level control
- Layer C++ wrappers that make remote memory feel like local objects

It is not a production-grade RDMA library. The focus is clarity and learning, not full correctness or safety in every edge case.

## What It Provides

- **C++ abstractions** that make remote RDMA memory feel like local objects:
  - `RDMAConnection` for connection setup/cleanup
  - `RDMAVector` for a remote `int` array
  - `RDMAVariable<T>` and type aliases like `RDMAInt`, `RDMADouble` for remote scalars
- **C core layer** providing underlying RDMA infrastructure (device discovery, QP setup, verbs)

## Architecture

1. **Core (C)**: device discovery, QP setup, MR registration (read+write), and verbs calls
2. **C++ layer**: thin wrapper with operator overloading for natural syntax
3. **Example**: `rdma-example.cpp` demonstrating the abstractions

## Project Structure

```
.
├── include/                # Public header files
│   ├── rdma/               # C layer headers
│   │   └── common.h        # Core RDMA definitions and verbs wrappers
│   └── rdmacpp/           # C++ abstractions
│       ├── rdma_connection.hpp   # Connection management
│       ├── rdma-vector.hpp       # Remote vector abstraction
│       └── rdma-variable.hpp     # Remote variable abstraction
├── src/                   # Implementation files
│   ├── core/              # Core C implementation
│   │   └── common.c       # RDMA resource management and verbs
│   ├── cpp/               # C++ wrappers
│   │   ├── rdma_connection.cpp
│   │   └── rdma-vector.cpp
│   └── examples/          # Example applications
│       └── rdma-example.cpp   # Showcase of C++ abstractions
├── bin/                   # Output directory for executables
├── lib/                   # Output directory for libraries
├── obj/                   # Output directory for object files
├── LICENSE                # GNU AGPL v3 license
├── Makefile               # Build system
└── README.md              # This file
```

## Requirements

- Linux or Unix-based OS with libibverbs
- RDMA-capable hardware and RDMA stack (OFED or equivalent)
- GCC with C++11 support

## Building

```bash
# Install dependencies (Ubuntu/Debian example)
sudo apt-get install libibverbs-dev

# Build
make

# This creates bin/rdma-example (and bin/rdma as a symlink)

# Clean build artifacts
make clean
```

## Usage Example

```cpp
#include <iostream>
#include "rdmacpp/rdma_connection.hpp"
#include "rdmacpp/rdma-variable.hpp"
#include "rdmacpp/rdma-vector.hpp"

int main() {
    try {
        // Server side
        RDMAConnection server(MODE_RW);
        
        // Client side
        RDMAConnection client("server_hostname", MODE_RW);
        
        // Get remote memory region info (from connection negotiation)
        qp_info_t remote_info = client.get_remote_mr_info();
        
        // Create RDMA-ified variables pointing to remote memory
        RDMAVector vec(&client, remote_info.addr, remote_info.rkey, 10);
        RDMAInt counter(&client, remote_info.addr + 40, remote_info.rkey);
        
        // Use them like regular C++ objects - reads and writes work transparently
        vec[3] = 42;        // RDMA write
        int val = vec[3];   // RDMA read
        counter += 10;      // RDMA read + write
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

Run the example:
```bash
# On the server
./bin/rdma server

# On the client
./bin/rdma client <server_hostname>
```

## Notes and Limitations

- Remote "variables" are just offsets into a single registered buffer on the server.
- The buffer size is fixed at `MAX_BUFFER_SIZE` (4096 bytes), so all variables must fit within it.
- `RDMAVariable` reads use RDMA READ and writes use RDMA WRITE.
- All connections use `MODE_RW` (both read and write enabled).
- The examples assume a shared understanding of remote memory layout (e.g., offsets).
- `RDMAVector` currently targets `int` elements only.
- The code is intentionally compact and does not aim to cover every production safety case.

## License

This project is licensed under the GNU Affero General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
