#version 400
in vec4 positions;
in vec2 uvsend;

uniform sampler2D colorTexture;
uniform sampler2D debug;
uniform bool useTexture;
uniform vec4 color;
uniform mat4 modelToWorld;

out vec4 newColor;

void main()
{
	//if(useTexture) 
	//newColor = texture2D(colorTexture, uvsend);
	newColor =  texture2D(debug, uvsend);
	newColor = vec4(newColor.z, newColor.y, newColor.x,1);
	//else newColor = color;
};