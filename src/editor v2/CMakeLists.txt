# Make sure the compiler can find include files from our Hello library. 
include_directories (${RADIAL_SOURCE_DIR}/engine) 
include_directories (${Radial_SOURCE_DIR}/engine/contrib) 
include_directories (${Radial_SOURCE_DIR}/engine/contrib/bullet)

# Make sure the linker can find the Hello library once it is built. 
link_directories (${RADIAL_BINARY_DIR}/engine)
link_directories (${RADIAL_BINARY_DIR}/game)
link_directories (${QT_LIBRARY_DIR})
link_directories (${FBX_LIBRARY_DIR})
add_definitions(-DFBXSDK_SHARED)

#ADD_DEFINITIONS( -Wall )

# Add executable called "helloDemo" that is built from the source files 
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found. 

file( GLOB CPP_FILES *.cpp )
file( GLOB PH_CPP_FILES re*.cpp )
#file( GLOB EXCLUDE_PCH rePlayer.cpp )
#LIST (REMOVE_ITEM PH_CPP_FILES ${EXCLUDE_PCH})

message(STATUS "PH_CPP_FILES=${PH_CPP_FILES}")

file( GLOB MODIFIER_CPP_FILES *Modifier.cpp )
file( GLOB MODIFIER_HEADER_FILES *Modifier.h )
file( GLOB SHADER_FILES ${Radial_PROJECT_DIR}/resource/shaders/* )
#LIST(APPEND CPP_FILES "main.cpp" "std.cpp")
file( GLOB HEADER_FILES *.h )

file( GLOB UI_FILES ui/* )

include_directories (${Radial_SOURCE_DIR}/engine) 
include_directories (${Radial_SOURCE_DIR}/game) 
include_directories (${Radial_SOURCE_DIR}/maker)
include_directories (${FBX_INCLUDE_DIR})

SET(QT_MIN_VERSION "4.8.2")
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)
#SET(QT_USE_QTTEST TRUE)

SET(editor_RESOURCES resources/resources.qrc)

QT4_ADD_RESOURCES(editor_RESOURCES_RCC ${editor_RESOURCES})

INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(MOC_FILES ${HEADER_FILES})
QT4_WRAP_UI(UI_HEADERS ${UI_FILES})

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})
SOURCE_GROUP("Moc Files" FILES ${MOC_FILES})
SOURCE_GROUP("Header Files\\UI" FILES ${UI_HEADERS})
SOURCE_GROUP("Header Files\\UI\\Resource" FILES ${editor_RESOURCES_RCC})
SOURCE_GROUP("Header Files\\Util\\" FILES "reSplitter.h")
SOURCE_GROUP("Source Files\\Util\\" FILES "reSplitter.cpp")

SOURCE_GROUP("Header Files\\Modifiers\\" FILES ${MODIFIER_HEADER_FILES})
SOURCE_GROUP("Source Files\\Modifiers\\" FILES ${MODIFIER_CPP_FILES})

ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


add_executable (maker ${CPP_FILES} ${HEADER_FILES} ${UI_HEADERS} ${MOC_FILES} ${editor_RESOURCES_RCC} ${SHADER_FILES})

if (MSVC)
   SET_SOURCE_FILES_PROPERTIES(${PH_CPP_FILES} PROPERTIES COMPILE_FLAGS "/Yustd.h")
   set_source_files_properties(std.cpp PROPERTIES COMPILE_FLAGS "/Ycstd.h")
endif(MSVC)

target_link_libraries (maker ${QT_LIBRARIES})

find_package(OpenGL)

target_link_libraries (maker opengl32) 
target_link_libraries (maker glu32)

target_link_libraries (maker game) 
target_link_libraries (maker engine)

target_link_libraries (maker optimized fbxsdk-2013.3)
target_link_libraries (maker debug fbxsdk-2013.3d)
