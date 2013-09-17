#include "reCamera.h"
#include "core.h"

#include "reShader.h"

reCamera::reCamera()
{
	_nearPlane = .1; _farPlane = 2000;
	_aspect = 4/3.0f;	
	transform.matrix = glm::rotate(reMat4(), -45.0f, reVec3(1, 0, 0)) * glm::translate(reMat4(), reVec3(0, 0, 150));
	updateNeeded = 1;
}

void reCamera::updateMatrices()
{
	projection = glm::perspective(45.0f, aspect(), nearPlane(), farPlane());
	view = glm::inverse(transform.matrix);
	updateNeeded = 0;
}

void reCamera::apply()
{
	if (updateNeeded)
	{
		updateMatrices();
	}
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(projection));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(view));
}

void reCamera::farPlane( float val )
{
	_farPlane = val;
	updateNeeded = TRUE;
}

float reCamera::farPlane() const
{
	return _farPlane;
}

float reCamera::nearPlane() const
{
	return _nearPlane;
}

void reCamera::nearPlane( float val )
{
	_nearPlane = val;
	updateNeeded = TRUE;
}

void reCamera::aspect( float val )
{
	_aspect = val;
	updateNeeded = TRUE;
}

float reCamera::aspect() const
{
	return _aspect;
}

void reCamera::updateShader( reShader* shader )
{
	if (updateNeeded)
	{
		updateMatrices();
	}
	pushMatrices(shader);
}


void reCamera::pushMatrices( reShader* shader )
{
	if (updateNeeded)
	{
		updateMatrices();
	}
	shader->setUniformMatrix(reShader::projMatrix, 1, glm::value_ptr(projection));
	shader->setUniformMatrix(reShader::viewMatrix, 1, glm::value_ptr(view));
}

reVec4 reCamera::viewport() const
{
	return _viewport;
}

void reCamera::viewport( reVec4 val )
{
	_viewport = val;
	aspect(val.z / val.w);
}

//////////////////////////////////////////////////////////////////////////

rePointCamera::rePointCamera()
{
	perspective(true);
	distance(10);
}

void rePointCamera::updateMatrices()
{
	transform.matrix =	glm::rotate(reMat4(), angles().y+lookAngles().y, reVec3(0,1,0)) *
						glm::rotate(reMat4(), angles().x+lookAngles().x, reVec3(1,0,0)) *
						glm::rotate(reMat4(), angles().z+lookAngles().z, reVec3(0,0,1));
	transform.matrix = glm::translate(reMat4(), lookAt()) * transform.matrix * glm::translate(reMat4(), reVec3(0, 0, distance()));
	if (perspective())
	{
		projection = glm::perspective(45.0f, aspect(), nearPlane(), farPlane());
	} 
	else
	{
		float dx = distance() * tan(glm::radians(45.0f));
		float dy = dx / aspect();
		projection = glm::ortho(-dx, dx, -dy, dy, .1f, 1000.0f);
	}
	view = glm::inverse(transform.matrix);
	updateNeeded = FALSE;
}

void rePointCamera::lookAt( reVec3 val )
{
	_lookAt = val; 
	updateNeeded = true;
}

reVec3 rePointCamera::lookAt() const
{
	return _lookAt;
}

float rePointCamera::distance() const
{
	return _distance;
}

void rePointCamera::distance( float val )
{
	_distance = val; 
	updateNeeded = true;
}

reVec3 rePointCamera::angles() const
{
	return _angles;
}

void rePointCamera::angles( reVec3 val )
{
	_angles = val;
	updateNeeded = true;
}

bool rePointCamera::perspective() const
{
	return _perspective;
}

void rePointCamera::perspective( bool val )
{
	_perspective = val;
	updateNeeded = true;
}

std::string rePointCamera::className()
{
	return "rePointCamera";
}

reVec3 rePointCamera::lookAngles() const
{
	return _lookAngles;
}

void rePointCamera::lookAngles( reVec3 val )
{
	_lookAngles = val;
}

void rePointCamera::toJson( reVar& val, int categories )
{
	reObject::toJson(val, categories);
	val["distance"] = distance();
	val["lookAt"] = lookAt();
	val["lookAngles"] = lookAngles();
	val["angles"] = angles();
	val["perspective"] = perspective();
}

void rePointCamera::loadJson( reVar& val )
{
	reObject::loadJson(val);
	if (!val.isMember("distance"))
	{
		return;
	}
	distance(val["distance"].asFloat());
	lookAt(val["lookAt"].asVec3());
	lookAngles(val["lookAngles"].asVec3());
	angles(val["angles"].asVec3());
	perspective(val["perspective"].asInt());
}
