#version 330 core
layout (location = 1) out vec4 FragColor;
layout (location = 6) out vec4 FragPosition;
layout (location = 7) out vec4 FragNormal;

in vec2 TexCoord;

uniform sampler2D WaterSampler;
uniform sampler2D WaterNormalSampler;
uniform sampler2D PositionSampler;
uniform sampler2D WaterAnimation;
uniform sampler2D FragColorMap;
uniform float frame;
layout (std140) uniform CameraBlock
{
	mat4 PV;
	vec3 camDir;
	vec3 camPos;
};

void main()
{
	vec4 waterSample = texture(WaterSampler, TexCoord);
	vec4 positionSample = texture(PositionSampler, TexCoord);
	float fragDepth = dot(camDir, positionSample.xyz - camPos);
	float waterDepth = dot(camDir, waterSample.xyz - camPos);
	if(fragDepth < waterDepth) discard;
	vec4 waterNormalSample = texture(WaterNormalSampler, TexCoord);
	vec4 currentColor = texture(FragColorMap, TexCoord);
	float invFrames = 1.0f / 64.0f;
	vec4 waterColor = texture(WaterAnimation, fract(waterSample.xz / 6.0f) * vec2(1.0f, invFrames) + vec2(0.0f, frame * invFrames));
	float alpha = clamp(waterSample.w * clamp((fragDepth - waterDepth) / 18.0f + 0.7f, 0.0f, 1.0f), 0.0f, 1.0f);
	FragColor = vec4(waterColor.xyz * alpha + currentColor.xyz * (1.0f - alpha), 1.0f);
	FragPosition = vec4(waterSample.xyz, alpha);
	FragNormal = vec4(waterNormalSample.xyz, 1.0f);
}