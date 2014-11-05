#version 400

in layout(location=0) vec3 position;
in layout(location=3) vec2 uvs;
in layout(location=6) ivec4 blendingIndex;
in layout(location=7) vec4 blendingWeight;
uniform mat4 viewToProjection;
uniform mat4 worldToView;
uniform mat4 modelToWorld;
uniform mat4 animationMatrices[100];
out vec2 uvsend;
void main()
{
	vec4 animatedPosition = vec4(0,0,0,0);
	if(blendingIndex.x >=0 || blendingIndex.y >= 0 || blendingIndex.z >= 0 || blendingIndex.w >= 0)
	{
	if( blendingIndex.x >= 0) animatedPosition += blendingWeight.x * animationMatrices[blendingIndex.x] * vec4(position, 1);
	if( blendingIndex.y >= 0) animatedPosition += blendingWeight.y * animationMatrices[blendingIndex.y] * vec4(position, 1);
	if( blendingIndex.z >= 0) animatedPosition += blendingWeight.z * animationMatrices[blendingIndex.z] * vec4(position, 1);
	if( blendingIndex.w >= 0) animatedPosition += blendingWeight.w * animationMatrices[blendingIndex.w] * vec4(position, 1);
	}
	else
	{
		animatedPosition = vec4(position,1);
	}
	vec4 newPosition = animatedPosition;
	gl_Position = viewToProjection * worldToView  * modelToWorld  * newPosition;
	uvsend = uvs;
};