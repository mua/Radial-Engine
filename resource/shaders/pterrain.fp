//uniform sampler2D splatMap;
//uniform sampler2D brushMap;
//uniform sampler2D normalMap;
//uniform sampler2D splatTiles[4];

varying vec4 wpos;
//varying vec4 acolor;
varying vec3 normal;
uniform vec2 cellSize;

uniform sampler2D splatTiles[4];

void main()
{
	/*
	vec4 color = texture2D(splatMap,gl_TexCoord[0].st);
	color *= texture2D(brushMap,gl_TexCoord[0].st);

	vec3 normal = normalize(texture2D(normalMap,gl_TexCoord[0].st).xyz);
	float lum = dot(normal, normalize(-vec3(0.5,-1, 0.5)))/2;
	lum += dot(normal, normalize(-vec3(-1.5,-1, -1.5)))/2;
	vec4 ambient = vec4(0.1, 0.1, 0.1, 1)*3;
	vec4 light = vec4(lum, lum, lum, 1) + ambient;
	*/
	vec2 tilePos = vec2(wpos.x / cellSize.x, wpos.z / cellSize.y);
	vec2 uv = vec2(tilePos.x - int(tilePos.x),  tilePos.y - int(tilePos.y));

	vec4 color1 = texture2D(splatTiles[0], uv);
	vec4 color2 = texture2D(splatTiles[2], uv) * abs(dot(vec3(1,0,0), normal))/2;
	vec4 color3 = texture2D(splatTiles[3], uv) * abs(dot(vec3(0,0,1), normal));
	//vec4 color3 = texture2D(splatTiles[2], vec2(wpos.x - int(wpos.x/10), wpos.z - int(wpos.z/10))/100);
	//gl_FragColor = vec4(normal.x, normal.y, normal.z, 1);

	vec3 light = normalize(vec3(0, -1, 0));
	vec3 light2 = normalize(vec3(1, -1, 1));
	float c=max(dot(light , normal), 0.0);
	c += max(dot(light2 , normal), 0.0)/2;
	c = clamp(c+0.3, 0, 1);
	vec4 acolor = vec4(c,c,c,1);
	//vec4 acolor = vec4(normal.xyz, 1);// vec4(c,c,c,1);

	acolor.w = 1;
	gl_FragColor = acolor *2 * mix(mix(color1, color2, color2.a), color3, color3.a); //vec4(c, c ,c, 1);
	gl_FragColor.a = 1;
	//gl_FragColor = texture2D(normalMap,gl_TexCoord[0].st);
}


