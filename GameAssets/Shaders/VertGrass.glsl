#version 330 core
layout (location = 0) in uint part32;
layout (location = 1) in uint part16;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out vec3 Tangent;

layout (std140) uniform CameraBlock
{
	mat4 PV;
	vec3 camDir;
	vec3 camPos;
};
uniform vec3 offset;
uniform float plane = 0.0f;

void main()
{
	uint rotation = part32 & 63u;
	uint shearF = (part32 >> 6u) & 63u;
	uint shearR = (part32 >> 12u) & 63u;
	uint size = (part32 >> 18u) & 3u;
	uint X = (part32 >> 20u) & 511u;
	uint Y = ((part32 >> 29u) & 7u) | ((part16 & 63u) << 3u);
	uint Z = (part16 >> 6u) & 511u;
	float variant = float(part16 >> 15u);

	const float sizeBase = 0.3f;
	const float sizeGrowth = 0.1f;
	float sizef = sizeBase + sizeGrowth * float(size);
	vec3 pos = vec3(
		float(X) / 16.0f,
		float(Y) / 16.0f,
		float(Z) / 16.0f
	);
	float shearFf = (float(shearF) / 64.0f) * 2.0f - 1.0f;
	float shearRf = (float(shearR) / 64.0f) * 2.0f - 1.0f;
	float rotf = float(rotation) * 0.09817477f;
	vec3 F = vec3(sin(rotf), shearFf, cos(rotf));
	vec3 R = vec3(F.z, shearRf, -F.x);
	float u = float(uint(gl_VertexID) & 1u);
	float v = float((uint(gl_VertexID) >> 1u) & 1u);
	
	vec3 A = mix(F, R, plane);
	vec3 N = vec3(-A.z, 0.0f, A.x);

	vec3 vertex = pos + offset + A * (u * 2.0f - 1.0f) * sizef + vec3(0.0f, v * sizef * 2.0f, 0.0f);
	Normal = normalize(N);
	Tangent = normalize(A);
	const float third = 1.0f / 3.0f;
	TexCoord = vec2(u * 0.5f + 0.25f, v * third + third * variant + third * 0.5f);
	FragPos = vertex;
	gl_Position = PV * vec4(vertex, 1.0);
}