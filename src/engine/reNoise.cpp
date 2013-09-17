#include "reNoise.h"

#define GLM_SWIZZLE 

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

vec3 mod289(vec3 x) {
	return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

vec2 mod289(vec2 x) {
	return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

vec3 permute(vec3 x) {
	return mod289(((x*34.0f)+1.0f)*x);
}

float snoise(vec2 v)
{
	const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
		0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
		-0.577350269189626,  // -1.0 + 2.0 * C.x
		0.024390243902439); // 1.0 / 41.0
	// First corner
	vec2 i  = floor(v + dot(v, vec2(C.y, C.y)) );
	vec2 x0 = v -   i + dot(i, vec2(C.x, C.x));

	// Other corners
	vec2 i1;
	//i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
	//i1.y = 1.0 - i1.x;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	// x0 = x0 - 0.0 + 0.0 * C.xx ;
	// x1 = x0 - i1 + 1.0 * C.xx ;
	// x2 = x0 - 1.0 + 2.0 * C.xx ;
	vec4 x12 = vec4(x0.x, x0.y, x0.x, x0.y) + vec4(C.x, C.x, C.z, C.z);
	x12.x -= i1.x;
	x12.y -= i1.y;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

	vec3 m = max(0.5f - vec3(dot(x0,x0), dot(vec2(x12), vec2(x12)), dot(vec2(x12.z, x12.w),vec2(x12.z, x12.w))), 0.0f);
	m = m*m ;
	m = m*m ;

	// Gradients: 41 points uniformly over a line, mapped onto a diamond.
	// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

	vec3 x = 2.0f * fract(p * reVec3(C.w, C.w, C.w)) - 1.0f;
	vec3 h = abs(x) - 0.5f;
	vec3 ox = floor(x + 0.5f);
	vec3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159f - 0.85373472095314f * ( a0*a0 + h*h );

	// Compute final noise value at P
	vec3 g;
	g.x  = a0.x  * x0.x  + h.x  * x0.y;
	g.y = a0.y * x12.x + h.y * x12.y;
	g.z = a0.z * x12.z + h.z * x12.w;
	return 130.0f * dot(m, g);
}

vec4 calculateVertex(vec4 pos)
{
	vec2 loc = vec2(pos.x+30, pos.z+20);
	pos.y = snoise(loc/50.0f)*5*snoise(loc/100.0f)-2;// - pos.x/3.0f;
	return pos;
}

vec3 calculateVertex(vec3 pos)
{
	return vec3(calculateVertex(vec4(pos.x, pos.y, pos.z, 1)));
}

vec3 getNormal(vec4 pos)
{
	float s = .01f;
	//vec3 tl = vec3(calculateVertex(pos + vec4( -s, 0, -s, 0)) - pos);  
	vec3 l =  vec3(calculateVertex(pos + vec4( -s, 0,  0, 0)) - pos); 
	//vec3 bl = vec3(calculateVertex(pos + vec4( -s, 0,  s, 0)) - pos);     
	vec3 t  = vec3(calculateVertex(pos + vec4(  0, 0, -s, 0)) - pos); 
	vec3 b =  vec3(calculateVertex(pos + vec4(  0, 0,  s, 0)) - pos); 
	//vec3 tr = vec3(calculateVertex(pos + vec4(  s, 0, -s, 0)) - pos); 
	vec3 r =  vec3(calculateVertex(pos + vec4(  s, 0,  0, 0)) - pos); 
	//vec3 br = vec3(calculateVertex(pos + vec4(  s, 0,  s, 0)) - pos);     
 
	vec3 n1 = cross(r, t);
	vec3 n2 = cross(l, b);
	vec3 n3 = cross(t, l);
	vec3 n4 = cross(b, r);
	
	//return normalize(n2);
	return normalize((normalize(n1)+normalize(n2)+normalize(n3)+normalize(n4))/4.0f);
}