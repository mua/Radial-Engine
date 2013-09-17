#ifndef core_h__
#define core_h__

#define GL_GLEXT_PROTOTYPES

#include <windows.h>
#include <gl/glew.h>

#define _USE_MATH_DEFINES
#include <math.h>

enum reRenderState {
	reRS_WIREFRAME = 1,
	reRS_SELECT = 2
};

#endif // core_h__