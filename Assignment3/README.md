# CSC 305 Assignment 3

A demonstration of procedural terrain generation in OpenGL using coherent noise algorithms.
[Assignment Description](https://github.com/ataiya/icg/wiki/Assignment%233a-Virtual-World)

![Alt text](/screen.jpg?raw=true)

This project was developed as a single C++ project as part of a Visual Studio 2015 solution.

# Requirements Completed

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
