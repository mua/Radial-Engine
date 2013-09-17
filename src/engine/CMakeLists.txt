# Create a library called "Hello" which includes the source file "hello.cxx". 
# The extension is already found. Any number of sources could be listed here. 

include_directories (${Radial_SOURCE_DIR}/engine) 
include_directories (${Radial_SOURCE_DIR}/engine/contrib) 
include_directories (${Radial_SOURCE_DIR}/engine/contrib/bullet) 
#include_directories (${Radial_SOURCE_DIR}/engine/contrib/json) 
#include_directories (${Radial_SOURCE_DIR}/engine/contrib/glm) 

SET (BULLET_PHYSICS_SOURCE_DIR ${Radial_SOURCE_DIR}/engine/contrib)

file( GLOB CPP_FILES *.cpp )
file( GLOB HEADER_FILES *.h)
file( GLOB_RECURSE BULLET_HEADER_FILES 
	contrib/bullet/*.h
	contrib/bullet/BulletCollision/*.h
	contrib/bullet/BulletDynamics/*.h	
	contrib/bullet/BulletMultiThreaded/*.h
	contrib/bullet/LinearMath/*.h
	contrib/bullet/vectormath/*.h)
file( GLOB_RECURSE BULLET_CPP_FILES 
	contrib/bullet/*.cpp
	contrib/bullet/BulletCollision/*.cpp
	contrib/bullet/BulletDynamics/*.cpp
	contrib/bullet/BulletMultiThreaded/*.cpp
	contrib/bullet/LinearMath/*.cpp
	contrib/bullet/vectormath/*.cpp)

set(CONTRIB_CPP_FILES contrib/jsoncpp.cpp contrib/xml/pugixml.cpp)

SOURCE_GROUP("Source Files\\Contrib" FILES ${CONTRIB_CPP_FILES})
SOURCE_GROUP("Source Files\\Contrib\\Bullet" FILES ${BULLET_CPP_FILES})
SOURCE_GROUP("Header Files\\Contrib\\Bullet" FILES ${BULLET_HEADER_FILES})

add_library (engine ${CPP_FILES} ${HEADER_FILES} ${CONTRIB_CPP_FILES} ${BULLET_CPP_FILES} ${BULLET_HEADER_FILES})

target_link_libraries (engine opengl32) 
target_link_libraries (engine glew32) 
target_link_libraries (engine glu32) 

