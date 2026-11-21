#version 330 core
layout (location = 0) out vec3 WaterPosition;
layout (location = 1) out vec3 WaterNormal;

in vec3 FragPos;
in vec4 ClipPos;

uniform vec3 offset;
uniform usampler3D fluidSampler;
uniform sampler2D renderFragPos;

layout (std140) uniform CameraBlock
{
	mat4 PV;
	vec3 camDir;
	vec3 camPos;
};

void main()
{
	vec3 d = normalize(FragPos - camPos);
	vec3 p = FragPos - offset;
	//float currentDepth = dot(camDir, FragPos - camPos);
	//float depthGrowth = dot(camDir, d);
	//vec2 screenPos = (ClipPos.xy / ClipPos.w) * 0.5f + 0.5f;
	//float depthCull = dot(camDir, texture(renderFragPos, screenPos).xyz - camPos);
	vec3 id = 1.0f / max(d, 0.000001f);
	vec3 n = vec3(0.0f);
	float waterLevel;
	vec3 c;
	for (int i = 0; i < 96; i++)
	{
		c = floor(p + d * 0.001f);
		waterLevel = float(texture(fluidSampler, ((c + 0.5f) / 32.0f).zyx).r & 0xffu) / 256.0f;
		if((p.y - floor(c.y + 0.5f)) < (waterLevel - 0.01f)) break;
		if(
			p.x < -0.1f || 
			p.y < -0.1f || 
			p.z < -0.1f || 
			p.x > 32.1f || 
			p.y > 32.1f || 
			p.z > 32.1f
		) {
			WaterPosition = camPos + d * 50000.0f;
			WaterNormal = n;
			gl_FragDepth = 0.999999f;
			return;
		} 
		vec3 pc = p - c;
		vec3 lm;
		lm.x = (abs(d.x) > 0.0001f) ? (((d.x > 0.0f) ? (1.0f - pc.x) : (pc.x)) / abs(d.x)) : (100000000.0f);
		lm.y = (abs(d.y) > 0.0001f) ? (((d.y > 0.0f) ? (1.0f - pc.y) : (pc.y - waterLevel)) / abs(d.y)) : (100000000.0f);
		lm.z = (abs(d.z) > 0.0001f) ? (((d.z > 0.0f) ? (1.0f - pc.z) : (pc.z)) / abs(d.z)) : (100000000.0f);
		if(lm.x < lm.y && lm.x < lm.z) 
		{
			p += d * lm.x * 1.0f;
			//currentDepth += depthGrowth * lm.x;
			n = vec3(-sign(d.x), 0.0f, 0.0f);
		}
		else
		if(lm.y < lm.z) 
		{
			p += d * lm.y * 1.0f;
			//currentDepth += depthGrowth * lm.y;
			n = vec3(0.0f, -sign(d.y), 0.0f);
		}
		else 
		{
			p += d * lm.z * 1.0f;
			//currentDepth += depthGrowth * lm.z;
			n = vec3(0.0f, 0.0f, -sign(d.z));
		}
	}

	uint topCell = texture(
	//sampler
	fluidSampler, 
	//tex coord
		(
			(
				//cell center
				c + 0.5f
				//cell center above this cell
				 + vec3(0.0f, 1.0f, 0.0f)
			) 
			//normalise coordinates
			/ 32.0f
		).zyx //reorder axes
	).r;
	
	//fix normal errors
	//if((p.y - floor(p.y + d.y * 0.001f)) > (waterLevel - 0.06f)) n = vec3(0.0f, 1.0f, 0.0f);
	bool sameType = (topCell & (0xffu << 8u)) == (1u << 8u);
	float topWaterLevel = float(topCell & 0xffu) / 256.0f;
	n = mix(
		vec3(0.0f, 1.0f, 0.0f), 
		n, 
		clamp(
			sameType ? (1.0f) : (waterLevel - p.y),
			0.0f, 
			1.0f
		)
		);
	WaterPosition = vec3(p + offset);
	WaterNormal = n;
	vec4 fragmentClipPosition = (PV * vec4(p + offset, 1.0));
	vec3 normalisedClipPos = fragmentClipPosition.xyz / fragmentClipPosition.w;
	gl_FragDepth = normalisedClipPos.z * 0.5f + 0.5f;
	/*
	UnlitColor = vec4(0.15f, 0.1f, 0.1f, 1.0f);
	FragColor = vec4(0.1f, 0.5f, 0.78f, 1.0f);
	FragPosition = p + offset;
	FragNormal = n;
	FragSpecular = vec4(1.0f);
	FragLight = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	*/	
}	