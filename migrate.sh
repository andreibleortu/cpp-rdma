#!/bin/zsh
# Migration script for cpp-rdma project restructuring
# Compatible with zsh and bash

# Exit on error
set -e

# Function to log messages
log() {
  echo "[MIGRATE] $1"
}

log "Starting migration process..."

# Create directory structure
log "Creating directory structure..."
mkdir -p include/rdma
mkdir -p include/rdmacpp
mkdir -p src/core
mkdir -p src/cpp
mkdir -p src/modes/lambda
mkdir -p src/modes/read
mkdir -p src/modes/write
mkdir -p src/modes/send-recv
mkdir -p src/examples
mkdir -p lib
mkdir -p bin
mkdir -p obj

# Function to copy files with error checking
copy_file() {
  if [ ! -f "$1" ]; then
    log "ERROR: Source file $1 does not exist"
    return 1
  fi
  
  log "Copying $1 to $2"
  cp "$1" "$2"
  
  if [ $? -ne 0 ]; then
    log "ERROR: Failed to copy $1 to $2"
    return 1
  fi
  
  return 0
}

# Move C headers
log "Moving C headers..."
copy_file common.h include/rdma/ || exit 1
copy_file send-receive/send_receive.h include/rdma/ || exit 1
copy_file rdma-read/rdma_read.h include/rdma/ || exit 1
copy_file rdma-write/rdma_write.h include/rdma/ || exit 1
copy_file lambda/lambda.h include/rdma/ || exit 1

# Move C++ headers
log "Moving C++ headers..."
copy_file rdma_connection.hpp include/rdmacpp/ || exit 1
copy_file rdma-variable.hpp include/rdmacpp/ || exit 1

# Process rdma-vector.cpp to extract header
log "Extracting header from rdma-vector.cpp..."
if [ ! -f "rdma-vector.cpp" ]; then
  log "ERROR: rdma-vector.cpp not found"
  exit 1
fi

# Create the header file
cat > include/rdmacpp/rdma-vector.hpp << 'EOF'
// Generated from rdma-vector.cpp
#ifndef RDMA_VECTOR_HPP
#define RDMA_VECTOR_HPP

#include <iostream>
#include <cstdint>
#include <stdexcept>
#include "rdma_connection.hpp"

EOF

# Extract class definition - more portable approach
awk 'BEGIN{flag=0} /class RDMAVector/,/^};/ {print; if($0 ~ /^};/) flag=1} flag{exit}' rdma-vector.cpp >> include/rdmacpp/rdma-vector.hpp

# Check if extraction succeeded
if ! grep -q "class RDMAVector" include/rdmacpp/rdma-vector.hpp; then
  log "ERROR: Failed to extract RDMAVector class definition"
  exit 1
fi

# Finish the header
echo "" >> include/rdmacpp/rdma-vector.hpp
echo "#endif // RDMA_VECTOR_HPP" >> include/rdmacpp/rdma-vector.hpp

log "Header extraction complete"

# Move C implementations
log "Moving C implementations..."
copy_file common.c src/core/ || exit 1
copy_file rdma.c src/core/ || exit 1
copy_file send-receive/send_receive.c src/modes/send-recv/ || exit 1
copy_file rdma-read/rdma_read.c src/modes/read/ || exit 1
copy_file rdma-write/rdma_write.c src/modes/write/ || exit 1
copy_file lambda/lambda_client.c src/modes/lambda/ || exit 1
copy_file lambda/lambda_server.c src/modes/lambda/ || exit 1
copy_file lambda-run.c src/examples/ || exit 1

# Move C++ implementations
log "Moving C++ implementations..."
copy_file rdma_connection.cpp src/cpp/ || exit 1
copy_file rdma-example.cpp src/examples/ || exit 1

# Create proper implementation file for rdma-vector
log "Creating implementation file for rdma-vector..."
cat > src/cpp/rdma-vector.cpp << 'EOF'
// Implementation file for RDMAVector class
#include "rdmacpp/rdma-vector.hpp"

// All implementation is in the header file
EOF

# Create gitignore to exclude build artifacts
log "Creating .gitignore file..."
cat > .gitignore << 'EOF'
# Build directories
/bin/
/lib/
/obj/

# Editor files
*.swp
*~
.DS_Store
.vscode/
.idea/

# Build artifacts
*.o
*.so
*.a
rdma
rdma-example
EOF

# Function to validate the migration
validate_migration() {
  local errors=0
  
  # Check critical files
  for file in include/rdma/common.h src/core/common.c include/rdmacpp/rdma-vector.hpp src/examples/rdma-example.cpp; do
    if [ ! -f "$file" ]; then
      log "ERROR: Critical file $file is missing"
      errors=$((errors+1))
    fi
  done
  
  # Check if the new Makefile exists
  if [ ! -f "Makefile" ]; then
    log "WARNING: Makefile has not been updated for the new structure"
  fi
  
  if [ $errors -eq 0 ]; then
    log "Validation passed: All critical files are in place"
    return 0
  else
    log "Validation failed: $errors errors found"
    return 1
  fi
}

log "Migration completed"
validate_migration

# Make the script executable (in case it wasn't already)
chmod +x migrate.sh

log "Next steps:"
log "1. Update #include paths in source files to use the new structure"
log "2. Verify the Makefile works with the new directory structure"
log "3. Run 'make clean && make' to test the build"
