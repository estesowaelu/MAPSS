#version 110
uniform vec3 eyePos;
uniform float power;
uniform float numLights;
uniform float invNumLights;
uniform float invNumLightsHalf;
uniform sampler2D lightsTex;
uniform float att;

varying float leadership;
varying float crowd;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec3 vLightDir;


vec3 PointLight( vec3 vertex, vec3 normal, vec3 eyeDirN, vec3 lightCenter, float lightRadius, vec3 color, float cutoff )
{
    vec3 lightDir	= lightCenter - vertex;
    float distance	= length( lightDir );
	distance *= distance;
    float d			= max( distance - lightRadius, 0.0 );
    lightDir		/= distance;
	
	float denom = d + 1.0;
    float attenuation = 1.0 / ( denom*denom );
    
    attenuation		= ( attenuation - cutoff ) / ( 1.0 - cutoff );
    attenuation		= max( attenuation, 0.0 );
    
    float diff		= max( dot( lightDir, normal ), 0.0 );
    return color * diff * 3.0 * attenuation;
}



void main()
{
	
	vec3 eyeDir			= eyePos - vVertex.xyz;
	vec3 eyeDirNorm		= normalize( eyeDir );
	float ppEyeDiff		= max( dot( vNormal, vLightDir ), 0.0 );
	float ppSpec		= pow( ppEyeDiff, 20.0 );
	float ppFres		= pow( 1.0 - ppEyeDiff, 2.0 );
	
	vec3 baitLighting = vec3( 0.0 );
	float index = invNumLightsHalf;
	for( float i=0.0; i<numLights; i+=1.0 ){
		vec4 pos		= texture2D( lightsTex, vec2( index, 0.25 ) );
		vec4 color		= texture2D( lightsTex, vec2( index, 0.75 ) );
		float radius	= pos.a;
		baitLighting    += PointLight( vVertex.xyz, vNormal, eyeDirNorm, pos.rgb, 1.0, color.rgb, att );
		index			+= invNumLights;
	}
	
	vec3 litRoomColor	= vec3( ppEyeDiff * 0.2 + 0.6 + ppSpec ) * ( vNormal.y * 0.5 + 0.5 );

	vec3 blueish		= vec3( 0.035, 0.5, 0.4 );
	vec3 darkRoomColor	= baitLighting;
	
	gl_FragColor.rgb	= mix( litRoomColor, darkRoomColor, power );
	gl_FragColor.a		= 1.0;
}



