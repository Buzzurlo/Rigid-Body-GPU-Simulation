#version 400

in vec3 fragColor;
in vec3 fragNorm;
in vec3 fragPos;
in vec4 fragLightPos;

uniform vec3 viewPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D centers;
uniform sampler2D vel;
uniform sampler2D shadowMap;

//uniform int blinn;
//uniform float shinyness;

out vec4 out_Color;

float LinearizeDepth(float depth)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 200.0; // TODO: Replace by the zFar  of your perspective projection
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

float computeShadow(in vec4 lightPos, in vec3 lightDir, in vec3 normal) {
	vec3 projCoord = lightPos.xyz / lightPos.w;		// -1 ; 1
	projCoord = projCoord * 0.5f + 0.5f;			//  0 ; 1
	float closeDepth = texture(shadowMap, projCoord.xy).r;
	float currDepth = projCoord.z;
	if(currDepth > 1.0f)
		return 0;
	float bias = 0.00007 * dot(lightDir, normal);
	//float shadow = currDepth > closeDepth + bias ? 1.0f : 0.0f;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	int soft = 2;
	for(int x = -soft; x <= soft; ++x)
	{
		for(int y = -soft; y <= soft; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoord.xy + vec2(x, y) * texelSize).r; 
			shadow += currDepth - bias > pcfDepth ? 1.0f : 0.0f;
		}    
	}
	shadow /= (2*soft + 1) * (2*soft + 1);

	return shadow;
}


void main() {
	vec3 norm = normalize(fragNorm);
	const int nLights = 3;
	vec3 lightPos[nLights] = {	vec3(-5, 0, 5),
								vec3(0, 0, 0),
								vec3(5, 0, -5) };
	vec3 lightColor[nLights] = {	vec3(1,0,0),
									vec3(0,1,0),
									vec3(0,0,1) };
	float lightStrength[nLights] = {1, 1, 1};

	out_Color = vec4(0);
	for(int i = 0; i < nLights; ++i) {
		// Ambient
		float ambientStrength = 0.1f;
		vec3 ambient = ambientStrength * lightStrength[i] * fragColor * lightColor[i];
  	
		// Diffuse
		vec3 lightDir = normalize(lightPos[i] - fragPos);
		float diff = max(dot(lightDir, norm), 0.0f);
		vec3 diffuse = diff * lightStrength[i] * lightColor[i] * fragColor;
    
		// Specular
		vec3 viewPos2 = inverse(view)[3].xyz;
		float specularStrength = 0.5f;
		vec3 viewDir = normalize(viewPos2 - fragPos);
		
		float shinyness = 0.2;
		vec3 halfwayDir = normalize(lightDir + viewDir); 
		float spec = pow(max(dot(norm, halfwayDir), 0.0), shinyness * 4);

		vec3 specular = spec * lightStrength[i] * lightColor[i];
        
		float dist_sq = dot(lightPos[i] - fragPos, lightPos[i] - fragPos);
		float atten = 1 / (1 + 0.01f * dist_sq);

		float shadow = computeShadow(fragLightPos, lightDir, norm);
		//float shadow = 0;
		//shadow = i == 0 ? shadow : 0.0f;
		vec3 result = ambient + (diffuse + specular) * atten * (1 - shadow);
		out_Color += vec4(result, 1.0f);
	}
}