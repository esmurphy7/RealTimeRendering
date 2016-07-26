// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in float VertexHeightModelspace;

// Ouput data
out vec3 color;
	
float map(float s, float a1, float a2, float b1, float b2)
{
    return b1 + (s-a1)*(b2-b1)/(a2-a1);
}

void main()
{	
	// Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 500.0f;		

	// assign heights for each texture
	float snowHeight = 3.0;
	float grassHeight = 1.5;
	float sandHeight = 0.0;

	// compute distance of current vertex to each texture height
	float snowDist = length(VertexHeightModelspace - snowHeight);
	float grassDist = length(VertexHeightModelspace - grassHeight);
	float sandDist = length(VertexHeightModelspace - sandHeight);

	float sumDist = snowDist + grassDist + sandDist;

	// define blending weights for each texture
	//float snowWeight = 1.0;
	float snowWeight = snowDist / sumDist;	
	snowWeight = clamp(snowWeight, 0.0, 1.0);

	//float grassWeight = 0.5;
	float grassWeight = grassDist / sumDist;
	grassWeight = clamp(grassWeight, 0.0, 1.0);

	//float sandWeight = 0.25;
	float sandWeight = sumDist / sandDist;
	sandWeight = clamp(sandWeight, 0.0, 1.0);

	// blend colors from textures based on weights
	vec4 snowColor = texture(iTextureArray, vec3(UV, 0)) * snowWeight;
	vec4 sandColor = texture(iTextureArray, vec3(UV, 1)) * sandWeight;
	vec4 grassColor = texture(iTextureArray, vec3(UV, 2)) * grassWeight;

	vec4 blendedColor = snowColor + sandColor + grassColor;
	//vec4 blendedColor = mix(snowColor, sandColor, 1.0);
	//blendedColor = mix(blendedColor, grassColor, 1.0);

	// Material properties	
	//vec3 MaterialDiffuseColor = vec3(1.0, 0.5, 0.0);
	vec3 MaterialDiffuseColor = blendedColor.rgb;
	//vec3 MaterialDiffuseColor = texture(iHeightMapTextureSampler, UV).rgb;
	vec3 MaterialAmbientColor = vec3(0.2,0.2,0.2) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	// Distance to the light
	float distance = length( iLightPosition_worldspace - Position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( Normal_cameraspace );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection_cameraspace );
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	color = 
		// Ambient : simulates indirect lighting
		MaterialAmbientColor +
		// Diffuse : "color" of the object
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		// Specular : reflective highlight, like a mirror
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
}