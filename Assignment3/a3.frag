// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in float VertexHeightModelspace;

// Ouput data
out vec3 color;

float calculateWeight(float dist, float range)
{
	float weight = 1.0 - (dist / range);
	weight = clamp(weight, 0.0, 1.0);
	return weight;
}

void main()
{	
	// Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 500.0f;		

	// assign heights for each texture
	float range = 2.0;
	float waterHeight = 0.0;
	float sandHeight = waterHeight + range;
	float grassHeight = sandHeight + range;		
	float rockHeight = grassHeight + range;
	float snowHeight = rockHeight + range;

	// compute distance of current vertex to each texture height
	float snowDist = length(VertexHeightModelspace - snowHeight);
	float rockDist = length(VertexHeightModelspace - rockHeight);
	float grassDist = length(VertexHeightModelspace - grassHeight);
	float sandDist = length(VertexHeightModelspace - sandHeight);
	float waterDist = length(VertexHeightModelspace - waterHeight);

	// define blending weights for each texture
	float snowWeight = calculateWeight(snowDist, range);
	float rockWeight = calculateWeight(rockDist, range);
	float grassWeight = calculateWeight(grassDist, range);
	float sandWeight = calculateWeight(sandDist, range);
	float waterWeight = calculateWeight(waterDist, range);

	// blend colors from textures based on weights
	vec4 snowColor = texture(iTextureArray, vec3(UV, 0)) * snowWeight;
	vec4 rockColor = texture(iTextureArray, vec3(UV, 1)) * rockWeight;
	vec4 sandColor = texture(iTextureArray, vec3(UV, 2)) * sandWeight;
	vec4 grassColor = texture(iTextureArray, vec3(UV, 3)) * grassWeight;
	vec4 waterColor = texture(iTextureArray, vec3(UV, 4)) * waterWeight;

	vec4 blendedColor = snowColor 
						+ rockColor 
						+ sandColor 
						+ grassColor 
						+ waterColor;

	// Material properties	
	//vec3 MaterialDiffuseColor = vec3(0.0, 0.0, 0.0);
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