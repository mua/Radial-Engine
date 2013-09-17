uniform sampler2D heightmap;

noperspective in vec2 tex_coord;
const ivec3 off = ivec3(-5,0,5);
const float scaleY = 100;
const float scaleX = (1500.0f/1024.0f);

void main()
{

	//vec2 tex_coord = gl_TexCoord[0].st;

    vec4 wave = texture(heightmap, tex_coord);
    float s11 = wave.x * scaleY;
    float s01 = textureOffset(heightmap, tex_coord, off.xy).x * scaleY; // left
    float s21 = textureOffset(heightmap, tex_coord, off.zy).x * scaleY; // right
    float s10 = textureOffset(heightmap, tex_coord, off.yx).x * scaleY; // bottom
    float s12 = textureOffset(heightmap, tex_coord, off.yz).x * scaleY; // top
    //vec3 va = normalize(vec3(size.xy,s21-s11));
    //vec3 vb = normalize(vec3(size.yx,s12-s10));

	/*
	vec3 va = normalize(vec3(-1, (s21-s01), 0)); 
	vec3 vb = normalize(vec3(0, (s12-s10), 1));
    vec4 bump = vec4( normalize(cross(va,vb)), s11 );
	*/

	vec3 vLeft =   vec3(-scaleX, (s01-s11),  0);
	vec3 vRight =  vec3( scaleX, (s21-s11),  0); 
	vec3 vTop =    vec3( 0, (s12-s11),  scaleX);
	vec3 vBottom = vec3( 0, (s10-s11), -scaleX);
	 
	vec3 n1 = normalize(-cross(vLeft, vBottom));
	vec3 n2 = normalize(-cross(vRight, vTop));
	vec3 n3 = normalize(cross(vLeft, vTop));
	vec3 n4 = normalize(cross(vRight, vBottom));

	vec4 bump = vec4(normalize((n1+n2+n3+n4)/4.0), s11);
	gl_FragColor =  vec4(bump.xyz, 1);
}
