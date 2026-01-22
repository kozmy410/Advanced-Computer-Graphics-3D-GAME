#version 330 core
// taking in the raw coordinates of the sun's shape (the vertices)
layout (location = 0) in vec3 aPos;

// these matrices are sent from c++ to handle 3d math
uniform mat4 projection; // defines the field of view (perspective)
uniform mat4 view;       // defines where the camera is
uniform mat4 model;      // defines where the sun is specifically located

void main()
{
    // we multiply the matrices together to find the final 2d pixel position on your monitor.
    // read it from right to left: 
    // 1. start with the point (aPos)
    // 2. move it into world space (model)
    // 3. shift it based on the camera (view)
    // 4. flatten it for the screen (projection)
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}