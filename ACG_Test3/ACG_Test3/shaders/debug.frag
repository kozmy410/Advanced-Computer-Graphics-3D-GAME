#version 330 core

out vec4 FragColor;

void main()
{
    // This shader ignores all uniforms and just outputs a solid,
    // unmistakable magenta color. If we see magenta, we know this worked.
    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Magenta
}