varying vec2 uv;
uniform sampler2D diffuseTexture;
uniform vec4 diffuseColor;
varying vec4 normal;
varying vec4 worldPosition;
varying vec4 viewPosition;

#ifdef _SHADOW_CAST_
#pragma include "vsm_cast.fp"
#else
float illumination()
{
	return 1.0;
}
#endif

#ifdef _PROC_TERRAIN_
#pragma include "procterrain.fp"
#else
vec4 getDiffuse()
{
	#ifdef _DIFFUSE_TEXTURE_
	return diffuseColor * texture2D(diffuseTexture, uv);
	#else
	return diffuseColor;
	#endif
}
#endif

#ifdef _SHADOW_MAP_
#pragma include "vsm_shadow.fp"
#else
vec4 getColor()
{
	#ifdef _SHADOW_CAST_
	float i = clamp(1-(1-illumination())/1.5, 0, 1.0);
	vec4 color = getDiffuse() * vec4(i, i, i, 1);
	if (color.a < 0.1) discard;
	//color.a = 1;
	return color;
	#else
	vec4 color = getDiffuse() * illumination();
	//color.a = 1;
	return color;
	#endif
}
#endif

void main()
{
	gl_FragColor = getColor();
}
