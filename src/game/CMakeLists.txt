# Make sure the compiler can find include files from our Hello library. 
include_directories (${RADIAL_SOURCE_DIR}/engine) 

# Make sure the linker can find the Hello library once it is built. 
link_directories (${RADIAL_BINARY_DIR}/engine) 

# Add executable called "helloDemo" that is built from the source files 
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found. 

add_library (game game.cpp)

include_directories (${Radial_SOURCE_DIR}/engine) 

# Link the executable to the Hello library. 

target_link_libraries (game engine) 