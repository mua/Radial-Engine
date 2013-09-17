# The name of our project is "HELLO". CMakeLists files in this project can 
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and 
# to the root binary directory of the project as ${HELLO_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.6) 
project (Radial) 

# Recurse into the "Hello" and "Demo" subdirectories. This does not actually 
# cause another cmake executable to run. The same process will walk through 
# the project's entire directory structure. 


SET(Radial_PROJECT_DIR C:/Users/utku/Desktop/cmake)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Radial_PROJECT_DIR}/bin/)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Radial_PROJECT_DIR}/bin/)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Radial_PROJECT_DIR}/bin/)
SET(EXECUTABLE_OUTPUT_PATH ${Radial_PROJECT_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${Radial_PROJECT_DIR}/bin/)

SET(QT_INCLUDE_DIR C:/dev/lib/Qt/4.8.2/include)
SET(QT_LIBRARY_DIR C:/dev/lib/Qt/4.8.2/lib)
SET(FBX_INCLUDE_DIR C:/dev/lib/fbx/2013.3/include)
SET(FBX_LIBRARY_DIR C:/dev/lib/fbx/2013.3/lib/vs2010/x86)

find_package(OpenGL)

add_subdirectory (engine) 
add_subdirectory (game)
add_subdirectory (editor) 
add_subdirectory (demo)
add_subdirectory (editor v2) 
#add_subdirectory (contrib/qpb)



