// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
  
void main()
{
  // Output position of the vertex, in clip space : iModelViewProjection * position
  gl_Position =  iModelViewProjection * vec4(vertexPosition_modelspace,1);
}