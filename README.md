# cpp-rdma

A high-performance RDMA library providing both C and C++ interfaces.

## Project Structure

```
.
├── include/               # Public headers
│   ├── rdma/              # C API headers
│   └── rdmacpp/           # C++ API headers
├── src/                   # Implementation files
│   ├── core/              # Core C implementation
│   ├── cpp/               # C++ wrappers
│   ├── modes/             # RDMA operation modes
│   │   ├── lambda/        # Remote function execution
│   │   ├── read/          # RDMA read operations
│   │   ├── send-recv/     # Two-sided communication
│   │   └── write/         # RDMA write operations
│   └── examples/          # Example applications
├── lib/                   # Output directory for libraries
├── bin/                   # Output directory for executables
├── LICENSE                # License file
├── Makefile               # Build system
└── README.md              # Project documentation
```

## Building

```bash
# Build everything
make

# Build only the C library
make rdma

# Build C++ wrappers and examples
make rdmacpp

# Clean build artifacts
make clean
```

## Examples

The library includes several examples demonstrating different RDMA operations:

- Two-sided send/receive
- One-sided RDMA read
- One-sided RDMA write
- Remote function execution (Lambda)
- C++ abstractions (RDMAVector, RDMAVariable)
