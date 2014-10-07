#version 400

in layout(location=0) vec3 position;
in layout(location=3) vec2 uvs;
uniform mat4 viewToProjection;
uniform mat4 worldToView;
uniform mat4 modelToWorld;
out vec2 uvsend;
out vec4 positions;
void main()
{
	vec4 newPosition = modelToWorld * vec4(position,1);
	gl_Position = viewToProjection * worldToView * newPosition;
	uvsend = uvs;
	positions = newPosition;
};