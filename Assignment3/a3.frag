// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

// Ouput data
out vec3 color;
	
void main()
{
	//color = vec3(1.0, 1.0, 1.0);
	color.r = texture( iTextureSampler, UV ).r;
	color.g = 1.0;
	color.b = 0.0;
}