#ifndef reCamera_h__
#define reCamera_h__

#include "reTransform.h"
#include "reTypes.h"
#include "reObject.h"
#include "reRenderer.h"

class reCamera: public reObject, public reRenderClient
{
public:
	float _aspect; 
	float aspect() const;
	void aspect(float val);

	reVec4 _viewport;
	reVec4 viewport() const;
	void viewport(reVec4 val);

	float _nearPlane;
	float nearPlane() const;
	void nearPlane(float val);

	float _farPlane;
	float farPlane() const;
	void farPlane(float val);

	reMat4 projection, view;

	reTransform transform;

	reCamera();
	bool updateNeeded;
	virtual void apply();
	virtual void updateMatrices();

	virtual void updateShader(reShader* shader) override;

	void pushMatrices( reShader* shader );

};

class rePointCamera:
	public reCamera
{
private:
	reVec3 _lookAt;
	reVec3 _angles;
	reVec3 _lookAngles;
	float _distance;
	bool _perspective;
public:
	rePointCamera();
	reVec3 lookAt() const;
	void lookAt(reVec3 val);
	float distance() const;
	void distance(float val);
	reVec3 angles() const;
	void angles(reVec3 val);
	reVec3 lookAngles() const;
	void lookAngles(reVec3 val);
	bool perspective() const;
	void perspective(bool val);

	virtual void updateMatrices();
	virtual string className();

	void toJson(reVar& val, int categories);

	void loadJson( reVar& val );

};
#endif // reCamera_h__