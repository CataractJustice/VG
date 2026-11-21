#version 330 core

out vec3 FragPos;
out vec4 ClipPos;

layout (std140) uniform CameraBlock
{
	mat4 PV;
	vec3 camDir;
	vec3 camPos;
};
uniform vec3 offset;
uniform float screenspace;

const uint indices[36] = 
uint[36](
	0u, 2u, 3u,
	3u, 1u, 0u,
	1u, 3u, 7u,
	7u, 5u, 1u,
	5u, 7u, 6u,
	6u, 4u, 5u,
	4u, 6u, 2u,
	2u, 0u, 4u,
	2u, 6u, 7u,
	7u, 3u, 2u,
	0u, 1u, 5u,
	5u, 4u, 0u
);
void main()
{
	float inv15 = 1.0f / 15.0f;

	vec3 vertices[8] = vec3[8](
		vec3(0.0f, 0.0f, 0.0f),
		vec3(1.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(1.0f, 0.0f, 1.0f),
		vec3(0.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f)
	);

	vec3 vertice = vertices[indices[gl_VertexID]];
	vec3 vertexPosition = (vertice * 32.0f + offset) * (1.0f - screenspace) + (vertice * -0.5f + 0.25f + camPos) * (screenspace);
	
	FragPos = vertexPosition;
	gl_Position = PV * vec4(vertexPosition, 1.0);
	ClipPos = gl_Position;
}