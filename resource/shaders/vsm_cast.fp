uniform sampler2D shadowMap;
uniform mat4 lightMatrix[2];
uniform mat4 lightViewMatrix[2];
uniform mat4 lightProjMatrix[2];
uniform mat4 biasMatrix;
uniform float lightNearDistance;
uniform float lightFarDistance;

varying vec4 lightViewPosition;

vec4 ShadowCoordPostW;

float chebyshevUpperBound( float distance)
{
	//float factor = lightProjMatrix[0][3][2] - lightProjMatrix[0][3][3];
	vec2 moments = texture2D(shadowMap, ShadowCoordPostW.xy).rg;// * factor;
	
	// Surface is fully lit. as the current fragment is before the light occluder
	if (distance <= moments.x)
		return 1.0 ;

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance, 0.00000002);

	float d = distance - moments.x;
	float p_max = variance / (variance + d*d);
	return p_max;

	float val = smoothstep(0.3, 1.0, p_max);
	if (val > 0.8) val = 1;
	return val;
}

float illumination()
{
	vec4 tpos = biasMatrix * lightMatrix[0] * worldPosition;
	vec4 vpos = lightViewPosition / lightViewPosition.w;
	tpos = tpos / tpos.w;
	ShadowCoordPostW = tpos;
	
	float shadow;
	float distance = ((-vpos.z)-lightNearDistance)/(lightFarDistance-lightNearDistance);
	/*
	shadow = chebyshevUpperBound(distance);

	vec4 ldir = vec4(normalize(normal));
	ldir.w = 0; 
	ldir = lightViewMatrix[0] * ldir;
	return min(shadow, ldir.z);
	*/

    float shadowed = 0.0;
	shadow = 0;

	int i = 0;
    for(float y = -1.5; y<=1.5; y+=1.0)
	{
        for(float x = -1.5; x<=1.5; x+=1.0)
		{
			ShadowCoordPostW.xy = tpos.xy + vec2(x, y) / vec2(1024, 1024);
			shadow += chebyshevUpperBound(distance);
			i++;
		}
	}
	vec4 ldir = vec4(normalize(normal));
	ldir.w = 0; 
	ldir = lightViewMatrix[0] * ldir ;
	return min((shadow / i), ldir.w !=0 ? ldir.z/ldir.w:ldir.z);
}
