#version 330 core
layout (location = 0) out vec4 UnlitColor;
layout (location = 1) out vec4 FragColor;
layout (location = 2) out vec3 FragPosition;
layout (location = 3) out vec3 FragNormal;
layout (location = 4) out vec4 FragSpecular;
layout (location = 5) out vec4 FragLight;

in vec3 FragPos;
in vec4 ClipPos;
in vec4 QuadUV;
in vec3 Normal;
in vec3 XAxis;
in vec3 YAxis;
in vec3 Offset;

uniform sampler2D textureImage;
uniform usampler3D materialVolume;
uniform float atlasScaleInv;
uniform vec4 spec = vec4(1.0f);

const float tileWorldOffset = 0.5f;

vec4 getTextureValue(vec3 w, vec2 UVx, vec2 UVy, vec2 UVz, vec2 textureOffset, vec2 topTextureOffset) 
{	
	vec4 t0 = texture(textureImage, UVx + textureOffset);
	vec4 t1 = texture(textureImage, UVy + textureOffset);
	vec4 t1top = texture(textureImage, UVy + topTextureOffset);
	vec4 t2 = texture(textureImage, UVz + textureOffset);

	return 	t0 * w.x + 
			mix(t1top, t1, step(Normal.y, 0.0f)) * w.y + 
			t2 * w.z; 
}

vec3 getnormalValue(vec3 w, vec2 UVx, vec2 UVy, vec2 UVz, vec2 textureOffset, vec2 topTextureOffset) 
{
	vec3 n0 = texture(textureImage, UVx + textureOffset).xyz;
	vec3 n1 = texture(textureImage, UVy + textureOffset).xyz;
	vec3 n1top = texture(textureImage, UVy + topTextureOffset).xyz;
	vec3 n2 = texture(textureImage, UVz + textureOffset).xyz;

	n0 = vec3(
	n0.xy + Normal.zy,
	abs(n0.z) * Normal.x
	);	

	n1 = vec3(
	n1.xy + Normal.xz,
	abs(n1.z) * Normal.y
	);

	n1top = vec3(
	n1top.xy + Normal.xz,
	abs(n1top.z) * Normal.y
	);

	n2 = vec3(
	n2.xy + Normal.xy,
	abs(n2.z) * Normal.z
	);

	return normalize(
		n0.zyx * w.x +
		mix(n1top.xzy, n1.xzy, step(Normal.y, 0.0)) * w.y +
		n2.xyz * w.z
	);
}

void main()
{
	vec2 texOffset = vec2(0.0f, 2.0f * atlasScaleInv);
	vec3 w = abs(Normal);
	//=====================================================
	vec3 FragPosOffset = FragPos - Offset;
	float FragUVX = dot(XAxis, FragPosOffset);
	float FragUVY = dot(YAxis, FragPosOffset);
	//Fix face edges sampling material outside of the cell
	if(FragUVX < QuadUV.x + 0.5f) FragPosOffset += XAxis * 0.5f;
	if(FragUVX > (QuadUV.x + QuadUV.z - 0.5f)) FragPosOffset -= XAxis * 0.5f;
	if(FragUVY < QuadUV.y + 0.5f) FragPosOffset += YAxis * 0.5f;
	if(FragUVY > (QuadUV.y + QuadUV.w - 0.5f)) FragPosOffset -= YAxis * 0.5f;
	//=====================================================
	ivec3 MatPos = ivec3(floor(FragPosOffset - Normal * 0.5f));
	MatPos = ((MatPos % 32) + 32) % 32;
	float materialOffset = float(texelFetch(materialVolume, MatPos.zyx, 0).r) * 12.0f + 0.5f;
	vec2 UVx = (fract(FragPos.zy) + materialOffset) * vec2(1.0f, atlasScaleInv);
	vec2 UVy = (fract(FragPos.xz) + materialOffset) * vec2(1.0f, atlasScaleInv);
	vec2 UVz = (fract(FragPos.xy) + materialOffset) * vec2(1.0f, atlasScaleInv);
	FragColor = getTextureValue(w, UVx, UVy, UVz, texOffset * 0.0f, texOffset);
	UnlitColor = getTextureValue(w, UVx, UVy, UVz, texOffset * 4.0f, texOffset * 4.0f);
	FragNormal = getnormalValue(w, UVx, UVy, UVz, texOffset * 2.0f, texOffset * 3.0f);
	FragSpecular = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	FragPosition = FragPos;
	FragLight = vec4(0.0f);
}