#version 400
in vec4 positions;
in vec2 uvsend;
in vec4 normals;


uniform sampler2D colorTexture;
uniform bool useTexture;
uniform vec4 color;

uniform vec4 lightColor;
uniform vec3 lightPosition;

uniform mat4 lightView;
uniform mat4 lightProjection;
uniform sampler2D shadowTexture;

out vec4 newColor;

void main()
{
	//if(useTexture) 
	newColor = texture2D(colorTexture, uvsend);
	//else newColor = color;

	vec3 normalLight = normalize(lightPosition - vec3(positions));
	float diffusePercent = clamp(dot(normalLight, vec3(normals)), 0, 1);
	vec4 preInLight = lightProjection * lightView * positions;
	vec3 inLight = preInLight.xyz / preInLight.w;
	vec2 sampleUV = vec2(inLight.x, inLight.y);
	sampleUV =  (0.5 * sampleUV + vec2(0.5,0.5));
	float shadowCompare = texture2D(shadowTexture, sampleUV)[0];
	if( shadowCompare <= 0.5 * inLight.z + 0.5 || (sampleUV.x < 0 || sampleUV.x > 1 || sampleUV.y < 0 || sampleUV.y> 1))
	{
		diffusePercent *= 0.5;
	}

	newColor = clamp(((diffusePercent * lightColor)) * newColor,0,1);
};