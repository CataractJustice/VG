#version 330 core
layout (location = 0) out vec4 UnlitColor;
layout (location = 1) out vec4 FragColor;
layout (location = 2) out vec3 FragPosition;
layout (location = 3) out vec3 FragNormal;
layout (location = 4) out vec4 FragSpecular;
layout (location = 5) out vec4 FragLight;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;

uniform vec4 textureColor;
uniform vec4 unlitColor;
uniform vec3 specularColor;
uniform sampler2D unlit;
uniform sampler2D textureImage;
uniform sampler2D specular;
uniform sampler2D normals;

void main()
{
    UnlitColor = texture(unlit, TexCoord) * unlitColor;
    FragColor = texture(textureImage, TexCoord) * textureColor;
	if((UnlitColor.a == 0.0f && FragColor.a < 0.8f)) 
	{
		discard;
	}
	FragPosition = FragPos;
	UnlitColor.a = 1.0f;
	FragColor.a = 1.0f;
	vec3 ln = texture(normals, TexCoord).xyz * 2.0f - 1.0f;
	FragNormal = normalize((gl_FrontFacing ? Normal : -Normal) * ln.z + vec3(0.0f, ln.y + 0.4f, 0.0f) + ((gl_FrontFacing ? Tangent : -Tangent) * ln.x));
	FragSpecular = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	FragLight = vec4(0.0f);
}