# Make sure the compiler can find include files from our Hello library. 
include_directories (${RADIAL_SOURCE_DIR}/engine) 
include_directories (${Radial_SOURCE_DIR}/engine/contrib) 

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
file( GLOB HEADER_FILES *.h )

file( GLOB UI_FILES ui/* )
#SET(UI_FILES ui/terrain.ui)

file( GLOB ASSET_HEADER_FILES re*Asset.h )
file( GLOB ASSET_CPP_FILES re*Asset.cpp )
file( GLOB SHADER_FILES assets/* )

include_directories (${Radial_SOURCE_DIR}/engine) 
include_directories (${Radial_SOURCE_DIR}/game) 
include_directories (${Radial_SOURCE_DIR}/editor)
#include_directories (${Radial_SOURCE_DIR}/contrib/qpb/src)
include_directories (${FBX_INCLUDE_DIR})

SET(QT_MIN_VERSION "4.8.2")
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)

SET(editor_RESOURCES resources/resources.qrc)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})
#SOURCE_GROUP("Sources" FILES ${CPP_FILES})
SOURCE_GROUP("Source Files\\Assets" FILES ${ASSET_CPP_FILES})

#SOURCE_GROUP("Headers" FILES ${HEADER_FILES})
SOURCE_GROUP("Header Files\\Assets" FILES ${ASSET_HEADER_FILES})

SOURCE_GROUP("Header Files\\Context" FILES reContext.h reContextWindow.h reSceneGraph.h reViewport.h 
  reContextModel.h reViewset.h reTerrainPanel.h)
SOURCE_GROUP("Source Files\\Context" FILES reContext.cpp reContextWindow.cpp reSceneGraph.cpp reViewport.cpp 
  reContextModel.cpp reViewset.cpp reTerrainPanel.cpp)

SOURCE_GROUP("Header Files\\Helpers" FILES reHelper.h reGizmo.h)
SOURCE_GROUP("Source Files\\Helpers" FILES reHelper.cpp reGizmo.cpp)

SOURCE_GROUP("Header Files\\Modifiers" FILES reModifier.h reSelectModifier.h reTransformModifier.h reTerrainModifier.h)
SOURCE_GROUP("Source Files\\Modifiers" FILES reModifier.cpp reSelectModifier.cpp reTransformModifier.cpp reTerrainModifier.cpp)

SOURCE_GROUP("Header Files\\Editors" FILES rePropertyEditor.h rePropertySet.h)
SOURCE_GROUP("Source Files\\Editors" FILES rePropertyEditor.cpp rePropertySet.cpp)

SOURCE_GROUP("Header Files\\Models" FILES reCollectionModel.h)
SOURCE_GROUP("Source Files\\Models" FILES reCollectionModel.cpp)

SOURCE_GROUP("Header Files\\Terrain" FILES reMapBrush.h reTerrainNode.h reTextureDialog.h reTerrainMaterial.h)
SOURCE_GROUP("Source Files\\Terrain" FILES reMapBrush.cpp reTerrainNode.cpp reTextureDialog.cpp reTerrainMaterial.cpp)

QT4_ADD_RESOURCES(editor_RESOURCES_RCC ${editor_RESOURCES})

INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(MOC_FILES ${HEADER_FILES})
QT4_WRAP_UI(UI_HEADERS ${UI_FILES})

SOURCE_GROUP("Moc Files" FILES ${MOC_FILES})
SOURCE_GROUP("Header Files\\UI" FILES ${UI_HEADERS})

ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_executable (editor ${CPP_FILES} ${HEADER_FILES} ${UI_HEADERS} ${MOC_FILES} ${editor_RESOURCES_RCC} ${SHADER_FILES})

#TARGET_LINK_LIBRARIES(editor ${QT_LIBRARIES})

#FOREACH(item ${QT_LIBRARIES})
#  STRING(REGEX MATCH ".*\\.lib$" islib ${item})
#  STRING(REGEX MATCH ".*\\d4.lib$" isdebug ${item})
#  IF(islib)
#    IF(NOT isdebug)
#      LIST(APPEND MY_QT_LIBS ${item})
#    endIF(NOT isdebug)
#  ENDIF(islib)
#ENDFOREACH(item ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(editor ${QT_LIBRARIES})

#FILE(GLOB _ALL_FILES ${QT_INCLUDE_DIR} ${QT_INCLUDE_DIR}/*)
#FOREACH(_FILE ${_ALL_FILES})
#  IF(IS_DIRECTORY ${_FILE})
#    INCLUDE_DIRECTORIES(${_FILE})
#  ENDIF()
#ENDFOREACH()

# Link the executable to the Hello library. 

find_package(OpenGL)

target_link_libraries (editor opengl32) 
target_link_libraries (editor glu32)

target_link_libraries (editor game) 
target_link_libraries (editor engine)
target_link_libraries (editor engine)

target_link_libraries (editor optimized fbxsdk-2013.2)
target_link_libraries (editor debug fbxsdk-2013.2d)

