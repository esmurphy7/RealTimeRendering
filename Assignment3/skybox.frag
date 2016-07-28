
in vec3 TexCoords;

out vec3 color;

void main()
{    
    color = texture(iSkyBoxCubeTexture, TexCoords).rgb;

	//if(TexCoords.z < -1.0 || TexCoords.z > 1.0)
	//{
	//	color = vec3(1.0,0.0,0.0);
	//}
	//else
	//{
	//	color = TexCoords;
	//}	
}