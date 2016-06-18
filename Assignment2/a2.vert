// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;
  
// Output data ; will be interpolated for each fragment.
out vec2 UV;

void main()
{
  // Output position of the vertex, in clip space : iModelViewProjection * position
  gl_Position =  iModelViewProjection * vec4(vertexPosition_modelspace, 1);

  // The color of each vertex will be interpolated
  // to produce the color of each fragment
  UV = vertexUV;
}