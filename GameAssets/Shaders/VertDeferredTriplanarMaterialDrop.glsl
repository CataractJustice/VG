#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 4) in vec3 normal;

out vec3 FragPos;
out vec3 FragLocalPos;
out vec3 Normal;
out vec3 LocalNormal;

layout (std140) uniform CameraBlock
{
	mat4 PV;
	vec3 camDir;
    vec3 camPos;
};

layout (std140) uniform TransformBlock
{
	mat4 model;
	mat4 normalModel;
};

void main()
{
	FragLocalPos = position;
	FragPos = vec3(model * vec4(position, 1.0f));
	gl_Position = PV * model * vec4(position, 1.0f);
	Normal = mat3(normalModel) * normal;
	LocalNormal = normal;
}