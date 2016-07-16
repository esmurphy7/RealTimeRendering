// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
  
// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

void main()
{
	// Output position of the vertex, in clip space : MVP * position
	 // Offset the y position by the value of current texel's colour value ?
	vec4 textureColor = texture(SandTexture, vertexUV);

	// offset the height (y-position) of the vertex based on the texture's R color
    vec4 offset = vec4(vertexPosition_modelspace.x, textureColor.r, vertexPosition_modelspace.z, 1.0);
	gl_Position =  iModelViewProjection * offset;
	//gl_Position =  iModelViewProjection * vec4(vertexPosition_modelspace, 1);
	
	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (iModel * vec4(vertexPosition_modelspace,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( iView * iModel * vec4(vertexPosition_modelspace,1)).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( iView * vec4(iLightPosition_worldspace,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	
	// Normal of the the vertex, in camera space
	Normal_cameraspace = ( iView * iModel * vec4(vertexNormal_modelspace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;	
}