
in vec3 TexCoords;

out vec3 color;

void main()
{    
    color = texture(iSkyBoxCubeTexture, TexCoords).rgb;
}