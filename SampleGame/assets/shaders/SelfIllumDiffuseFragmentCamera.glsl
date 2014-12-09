#version 400
in vec4 positions;
in vec2 uvsend;
in vec4 normals;
uniform sampler2D colorTexture;
uniform sampler2D debug;
uniform bool useTexture;
uniform vec4 color;
uniform mat4 modelToWorld;
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
	//newColor =  texture2D(debug, uvsend);
	newColor = vec4(newColor.z, newColor.y, newColor.x,1);
	//else newColor = color;
	vec3 normalLight = normalize(lightPosition - vec3(positions));
	float diffusePercent =1;
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