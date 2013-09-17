uniform sampler2D splatMap;
uniform sampler2D brushMap;
uniform sampler2D normalMap;
uniform sampler2D splatTiles[4];

varying vec4 wpos;

void main()
{
	vec4 color = texture2D(splatMap,gl_TexCoord[0].st);
	color *= texture2D(brushMap,gl_TexCoord[0].st);

	vec3 normal = normalize(texture2D(normalMap,gl_TexCoord[0].st).xyz);
	float lum = dot(normal, normalize(-vec3(0.5,-1, 0.5)))/2;
	lum += dot(normal, normalize(-vec3(-1.5,-1, -1.5)))/2;
	vec4 ambient = vec4(0.1, 0.1, 0.1, 1)*3;
	vec4 light = vec4(lum, lum, lum, 1) + ambient;
	
	gl_FragColor = color * light;
	//gl_FragColor = texture2D(normalMap,gl_TexCoord[0].st);
}


