
layout(location = 4) in vec3 skyBoxPosition_modelspace;

out vec3 TexCoords;

void main()
{
    gl_Position =   iModelViewProjection * vec4(skyBoxPosition_modelspace, 1.0);  
    TexCoords = skyBoxPosition_modelspace;
} 