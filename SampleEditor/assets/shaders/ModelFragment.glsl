#version 400
in vec2 uvsend;

uniform sampler2D colorTexture;

out vec4 newColor;

void main()
{
	newColor = texture2D(colorTexture, uvsend);
};