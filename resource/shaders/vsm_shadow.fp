varying vec4 projPosition;
uniform mat4 projMatrix;
uniform float nearDistance;
uniform float farDistance;

vec4 getColor()
{
	float depth = ((-viewPosition.z / viewPosition.w)-nearDistance)/(farDistance-nearDistance);
	//depth = depth * 0.5 + 0.5;			//Don't forget to move away from unit cube ([-1,1]) to [0,1] coordinate system

	//if (depth<0) return 0;
	#ifdef _DIFFUSE_TEXTURE_
	if (getDiffuse().a < 0.01) discard;
	#endif
	float moment1 = depth;
	float moment2 = depth * depth;

	// Adjusting moments (this is sort of bias per pixel) using derivative
	float dx = dFdx(depth);
	float dy = dFdy(depth);
	//moment2 += 0.25*(dx*dx+dy*dy);	

	return vec4( moment1, moment2, 0.0, depth > 0 ? 1.0 : 0);
/*
	float depth = projPosition.z / projPosition.w ;
	depth = depth * 0.5 + 0.5;			//Don't forget to move away from unit cube ([-1,1]) to [0,1] coordinate system

	float moment1 = depth;
	float moment2 = depth * depth;

	// Adjusting moments (this is sawort of bias per pixel) using derivative
	float dx = dFdx(depth);
	float dy = dFdy(depth);
	//moment2 += 0.25*(dx*dx+dy*dy) ;
	

	return vec4( moment1,moment2, 0.0, 0.0 );
	*/
}