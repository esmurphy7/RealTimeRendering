// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;
	
void main()
{
	// Output color = color of the texture at the specified UV
	//color = vec3(1.0, 0.0, 0.0);
	color = texture( iTextureSampler, UV ).rgb;
}