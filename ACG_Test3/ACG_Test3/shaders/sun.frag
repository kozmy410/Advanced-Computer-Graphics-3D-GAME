#version 330 core
out vec4 FragColor; // the final color of this pixel on the screen

// these values are sent from c++ (usually a warm yellow or white)
uniform vec3 sunColor;
uniform float sunIntensity;

void main()
{
    // for the sun, we don't calculate shadows or complex pbr lighting.
    // we just want it to "glow" at a constant brightness regardless of its position.
    // this makes it look like a light source rather than a physical object being lit.
    vec3 color = sunColor * sunIntensity;
    
    // outputting the final color with 1.0 alpha (fully solid)
    FragColor = vec4(color, 1.0);
}