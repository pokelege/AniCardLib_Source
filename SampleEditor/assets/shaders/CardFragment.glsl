#version 400

in vec2 uvs;

uniform sampler2D tex;
out vec4 color;
void main()
{
	color = texture2D(tex, vec2(uvs.x,-uvs.y));
}