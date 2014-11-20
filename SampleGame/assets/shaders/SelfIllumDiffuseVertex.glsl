#version 400

in layout(location=0) vec3 position;
in layout(location=2) vec3 normal;
in layout(location=3) vec2 uvs;
in layout(location=4) vec3 tangent;
in layout(location=5) vec3 bitangent;
in layout(location=6) ivec4 blendingIndex;
in layout(location=7) vec4 blendingWeight;
uniform mat4 viewToProjection;
uniform mat4 worldToView;
uniform mat4 modelToWorld;
uniform mat4 extraModelToWorld;
uniform mat4 animationMatrices[100];
out vec2 uvsend;
out vec4 positions;
out vec3 normals;
void main()
{
	vec4 animatedPosition = vec4(0,0,0,0);
	vec3 animatedNormals = vec3(0,0,0);
	if(blendingIndex.x >=0 || blendingIndex.y >= 0 || blendingIndex.z >= 0 || blendingIndex.w >= 0)
	{
		if( blendingIndex.x >= 0)
		{
			animatedPosition += blendingWeight.x * animationMatrices[blendingIndex.x] * vec4(position, 1);
			animatedNormals += blendingWeight.x * mat3(animationMatrices[blendingIndex.x]) * normal;
		}
		if( blendingIndex.y >= 0)
		{
			animatedPosition += blendingWeight.y * animationMatrices[blendingIndex.y] * vec4(position, 1);
			animatedNormals += blendingWeight.y * mat3(animationMatrices[blendingIndex.y]) * normal;
		}
		if( blendingIndex.z >= 0)
		{
			animatedPosition += blendingWeight.z * animationMatrices[blendingIndex.z] * vec4(position, 1);
			animatedNormals += blendingWeight.z * mat3(animationMatrices[blendingIndex.z]) * normal;
		}
		if( blendingIndex.w >= 0)
		{
			animatedPosition += blendingWeight.w * animationMatrices[blendingIndex.w] * vec4(position, 1);
			animatedNormals += blendingWeight.w * mat3(animationMatrices[blendingIndex.w]) * normal;
		}
	}
	else
	{
		animatedPosition = vec4(position,1);
		animatedNormals = normal;
	}
	vec4 newPosition = animatedPosition;
	gl_Position = viewToProjection * worldToView  * modelToWorld  * newPosition;
	uvsend = uvs;
	positions = newPosition;
	normals = normalize(mat3(modelToWorld) * normalize(animatedNormals));
};