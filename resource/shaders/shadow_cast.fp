uniform sampler2D shadowMap;
uniform mat4 lightMatrix[2];
uniform mat4 lightViewMatrix[2];
uniform mat4 biasMatrix;
varying vec4 shadowCoord;

float illumination()
{
	vec4 tpos = shadowCoord / shadowCoord.w;
	vec4 vpos = lightViewMatrix[0] * worldPosition / 10e10;
	//tpos = tpos / tpos.w;
	vec2 uv = tpos.xy;
	/*
	if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
		return 0;	
	*/
	float depth = texture2D(shadowMap, uv.xy).r;
	//return (depth-0.86)/0.95;
	if (depth < tpos.z) return 0.5;
	return 1.0;
}


