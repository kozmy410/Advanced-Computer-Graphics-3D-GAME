#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord; 

uniform vec3 objectColor;   
uniform sampler2D u_Texture; 

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = texColor * vec4(objectColor, 1.0);
}