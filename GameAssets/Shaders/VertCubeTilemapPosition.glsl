#version 330 core
layout (location = 0) in uint pos;

layout (std140) uniform CameraBlock
{
	mat4 PV;
	vec3 camDir;
	vec3 camPos;
};

uniform uint s;
uniform vec3 offset;
void main()
{
	vec3 xaxis = vec3(0.0f);
	vec3 yaxis = vec3(0.0f);
	vec3 normal = vec3(0.0f);
	xaxis[s >> 2u] = 1.0f;
	yaxis[(s >> 1u) + 1u - (s >> 2u)] = 1.0f;
	normal[2u - (s >> 1u)] = float(((s % 2u) << 1u)) - 1.0f;
	uint flip = (((s + 1u) >> 1u) % 2u) << 1u;
	uint vx = (pos >> 0u)  & 31u;	
	uint vy = (pos >> 5u)  & 31u;
	uint vn = ((pos >> 10u) & 31u) + (s % 2u);
	uint vw = (pos >> 15u) & 31u;
	uint vh = (pos >> 20u) & 31u;

	uint corner = uint(gl_VertexID) & 3u;
	corner = (corner ^ flip) * (corner & 1u) + corner * (1u - (corner & 1u));
	
	uint uvx = uint(corner >> 1u) * (vw + 1u);
	uint uvy = uint((corner >> 1u) ^ (corner & 1u)) * (vh + 1u);
	vec3 vertexPosition = float(vx + uvx) * xaxis + float(vy + uvy) * yaxis + float(vn) * abs(normal) + offset;

	gl_Position = PV * vec4(vertexPosition, 1.0);
}