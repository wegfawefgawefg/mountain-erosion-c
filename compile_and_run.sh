#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
if ! cmake ..; then
    echo -e "${RED}CMake configuration failed. Exiting.${NC}"
    exit 1
fi

# Run make
echo "Building project..."
if ! make; then
    echo -e "${RED}Build failed. Exiting.${NC}"
    exit 1
fi

# If we've made it this far, the build was successful
echo -e "${GREEN}Build successful.${NC}"

cd ..

# Run the executable
echo "Running SDLOpenGLExample..."
./build/game

