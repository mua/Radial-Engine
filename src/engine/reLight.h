#include "reCamera.h"

class reLight: public rePointCamera
{
public:
	virtual string className();
	virtual void updateShader(reShader* shader) override;
};