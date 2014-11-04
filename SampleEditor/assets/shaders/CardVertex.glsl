#version 400

in layout(location = 0) vec3 position;
in layout(location = 3) vec2 uv;

uniform mat4 modelToWorld;
out vec2 uvs;
void main()
{
	gl_Position = modelToWorld * vec4(position,1);
	uvs = uv;
}