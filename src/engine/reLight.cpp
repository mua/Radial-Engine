#include "reLight.h"
#include "reShader.h"
#include "reTypes.h"

void reLight::updateShader( reShader* shader )
{
	if (updateNeeded)
	{
		updateMatrices();
	}
	reMat4 mat(projection*view);
	//shader->setUniformMatrix("light[0]", 1, glm::value_ptr(mat));
}

std::string reLight::className()
{
	return "reLight";
}
