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
out vec4 newColor;

void main()
{
	//if(useTexture) 
	newColor = texture2D(colorTexture, uvsend);
	//newColor =  texture2D(debug, uvsend);
	newColor = vec4(newColor.z, newColor.y, newColor.x,1);
	//else newColor = color;
	vec3 normalLight = normalize(lightPosition - vec3(positions));
	float diffusePercent = clamp(dot(normalLight, vec3(normals)), 0, 1);

	newColor = clamp(((diffusePercent * lightColor)) * newColor,0,1);
};