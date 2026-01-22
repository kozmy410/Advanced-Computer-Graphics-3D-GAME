#version 330 core
out vec4 FragColor; // the final color of the water pixel

in vec3 FragPos; // the pixel's position in the world
in vec3 Normal;  // the direction the water surface is facing
in vec3 ViewPos; // where the camera is located

uniform vec3 lightPos;   // position of the sun/light
uniform vec3 lightColor; // color of the light
uniform vec3 objectColor; // the base "tint" of the water

// this allows the water to "see" the skybox images for reflections
uniform samplerCube skybox; 

void main() {
    // normalizing vectors to ensure the math stays accurate (length of 1.0)
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // 1. SPECULAR (the bright white glint on top of waves)
    // we calculate how light bounces off the water toward your eye
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // 32.0 is the shininess
    vec3 specular = lightColor * spec * 0.8;

    // 2. REFLECTION
    // we calculate the reflection vector (where the camera would see if the water were a mirror)
    vec3 I = normalize(FragPos - ViewPos);
    vec3 R = reflect(I, norm);
    
    // sampling the skybox using that reflection vector
    vec3 reflectionColor = texture(skybox, R).rgb;

    

    // 3. COLOR MIXING
    // we mix the deep water color with the sky reflection
    // 0.5 means it's a 50/50 blend between the texture and the tint
    vec3 waterBase = mix(objectColor, reflectionColor, 0.5); 
    
    // adding some basic diffuse lighting so the water reacts to light direction
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 finalColor = waterBase * (0.6 + 0.4 * diff);
    
    // adding the sun glint on top
    finalColor += specular;

    // 4. TRANSPARENCY
    // 0.8 alpha means the water is 80% solid, allowing you to see a bit of the world behind it
    FragColor = vec4(finalColor, 0.8); 
}