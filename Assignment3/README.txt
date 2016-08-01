CSC 305 Assignment 3 ==================================
This project was developed as a single C++ project as part of a Visual Studio 2015 solution.

Requirements Completed ================================
Geometry
- Generate noise on CPU and upload as texture
- Generate fBm noise
- Display scene with MVP matrices
- Use fBm texture as a displacement map
- Color terrain according to height
- Generate noise with hybrid multifractal (advanced)
- Generate noise with diamond-square algorithm (advanced)

Rendering
- Light terrain with Phong shading
- Texture terrain with blended tiles based on height
- Surround scene with Skybox
- Implement Skybox with OpenGL's CubeMap object (advanced)

Animation
- Implement flymode camera with keyboard and mouse
- Implement bezier curve construction (advanced)
- Animate camera path along a bezier curve (advanced)
- Concatenate multiple bezier curves to form a bezier path (advanced)

Branch Descriptions =====================================
master: Hybridmultifractal generation with bezier curve camera path
demo-fbm: fractal brownian motion generation
demo-diamond-square: terrain generation with diamond-square algorithm
demo-texture-blending: terrain is a linear, angular plane to demonstrate texture blending based on height
