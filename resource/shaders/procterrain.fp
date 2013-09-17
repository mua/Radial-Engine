uniform sampler2D splatTextures[4];
vec4 getDiffuse()
{
	vec4 color1 = texture2D(splatTextures[0], uv);
	return color1;
	vec4 color2 = texture2D(splatTextures[2], uv) * abs(dot(vec3(1,0,0), normal))/2;
	vec4 color3 = texture2D(splatTextures[3], uv) * abs(dot(vec3(0,0,1), normal));
	return mix(mix(color1, color2, color2.a), color3, color3.a);
}
