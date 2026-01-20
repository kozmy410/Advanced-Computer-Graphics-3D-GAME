#version 330 core

// We only take the position attribute. No matrices!
layout (location = 0) in vec3 aPos;

void main()
{
    // This is the simplest possible vertex shader.
    // It takes the vertex position and passes it directly to the screen.
    // The GPU treats these coordinates as "Normalized Device Coordinates" (-1 to 1 range).
    // Our square is defined from -0.5 to 0.5, so it should appear in the middle.
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}