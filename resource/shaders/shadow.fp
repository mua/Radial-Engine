varying vec4 projPosition;

vec4 getColor()
{
	float depth = projPosition.z / projPosition.w ;
	depth = depth * 0.5 + 0.5;
	return vec4(abs(depth),0,0,1);
}