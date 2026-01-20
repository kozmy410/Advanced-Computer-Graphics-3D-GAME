#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord; // Receive texture coordinates

uniform vec3 objectColor;   // For tinting (green for player, red for NPCs)
uniform sampler2D u_Texture; // The actual texture sampler

void main()
{
    // Get the color from the texture at the specific coordinate
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // Multiply the texture color by our tint color.
    // For the map, we'll use a white tint (1,1,1) so it's unchanged.
    // For the player, we'll tint our white square green.
    FragColor = texColor * vec4(objectColor, 1.0);
}