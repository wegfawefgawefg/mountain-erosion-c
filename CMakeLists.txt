cmake_minimum_required(VERSION 3.10)
project(FullScreenShader C)
set(CMAKE_C_STANDARD 11)

# Find SDL2 package
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find OpenGL package
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# Add the src directory to the include path
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add all source files in the src directory
file(GLOB SOURCES "src/*.c")

# Add the executable
add_executable(game ${SOURCES})

# Link the libraries
target_link_libraries(game ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

# On some systems, you might need to link to m (math library)
if(UNIX AND NOT APPLE)
    target_link_libraries(game m)
endif()