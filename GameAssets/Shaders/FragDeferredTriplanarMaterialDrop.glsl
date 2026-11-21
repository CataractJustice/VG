#version 330 core
layout (location = 0) out vec4 UnlitColor;
layout (location = 1) out vec4 FragColor;
layout (location = 2) out vec3 FragPosition;
layout (location = 3) out vec3 FragNormal;
layout (location = 4) out vec4 FragSpecular;
layout (location = 5) out vec4 FragLight;

in vec3 FragPos;
in vec3 FragLocalPos;
in vec3 Normal;
in vec3	LocalNormal;

uniform sampler2D textureImage;
uniform uint materialId;
uniform float texScale;
uniform vec2 atlasScale;

vec4 getTextureValue(vec3 w, float materialOffset, vec2 textureOffset, vec2 topTextureOffset) 
{
	vec2 UVx = (fract(FragLocalPos.zy / texScale) + materialOffset) / atlasScale;
	vec2 UVy = (fract(FragLocalPos.xz / texScale) + materialOffset) / atlasScale;
	vec2 UVz = (fract(FragLocalPos.xy / texScale) + materialOffset) / atlasScale;
	
	vec4 t0 = texture(textureImage, UVx + textureOffset);
	vec4 t1 = texture(textureImage, UVy + textureOffset);
	vec4 t1top = texture(textureImage, UVy + topTextureOffset);
	vec4 t2 = texture(textureImage, UVz + textureOffset);

	return t0 * w.x + ((LocalNormal.y > 0.0f) ? (t1top * w.y) : (t1 * w.y)) + t2 * w.z; 
}

vec3 getNormalValue(vec3 w, float materialOffset, vec2 textureOffset, vec2 topTextureOffset) 
{
	vec2 UVx = (fract(FragLocalPos.zy / texScale) + materialOffset) / atlasScale;
	vec2 UVy = (fract(FragLocalPos.xz / texScale) + materialOffset) / atlasScale;
	vec2 UVz = (fract(FragLocalPos.xy / texScale) + materialOffset) / atlasScale;
	
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
		(LocalNormal.y < 0.0f ? (n1.xzy) : (n1top.xzy)) * w.y +
		n2.xyz * w.z
	);
}

void main()
{
	vec2 texOffset = vec2(0.0f, 2.0f / atlasScale.y);
	float w0 = abs(LocalNormal.x * LocalNormal.x * LocalNormal.x);
	float w1 = abs(LocalNormal.y * LocalNormal.y * LocalNormal.y);
	float w2 = abs(LocalNormal.z * LocalNormal.z * LocalNormal.z);
	vec3 w = vec3(w0, w1, w2);
	float materialBaseTextureOffset = float(materialId) * 12.0f + 0.5f;

	FragColor = getTextureValue(w, materialBaseTextureOffset, texOffset * 0.0f, texOffset);
	UnlitColor = getTextureValue(w, materialBaseTextureOffset, texOffset * 4.0f, texOffset * 4.0f);
	FragNormal = getNormalValue(w, materialBaseTextureOffset, texOffset * 2.0f, texOffset * 3.0f);
	FragSpecular = vec4(0.0f, 1.0f, 1.0f, 1.0f);//getTextureValue(w, materialBaseTextureOffset, texOffset * 5.0f, texOffset * 5.0f);
	FragPosition = FragPos;
}