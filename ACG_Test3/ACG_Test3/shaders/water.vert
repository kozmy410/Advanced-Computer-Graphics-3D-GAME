#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// passing these to the fragment shader for lighting and reflections
out vec3 FragPos;
out vec3 Normal;
out vec3 ViewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float time; // the secret ingredient for movement!

// settings we can tweak from our c++ code or a gui
uniform float waveAmplitude; // how high the waves go
uniform float waveFrequency; // how many waves there are (tightness)
uniform float waveSpeed;     // how fast they move

// this helper calculates a single rolling wave in a specific direction
float CalculateWave(vec3 p, vec2 direction, float speedOffset, float freqOffset) {
    // finding the "distance" along the direction the wave is traveling
    float x = dot(p.xz, normalize(direction));
    
    // the math for a standard sine wave: sin(position + time)
    float t = time * waveSpeed * speedOffset;
    float f = waveFrequency * freqOffset;
    return sin(x * f + t) * waveAmplitude;
}

void main() {
    vec3 pos = aPos;
    
    // --- "OLD SCHOOL" INTERFERENCE WATER ---
    // if we use just one wave, it looks like a boring sheet of metal.
    // by adding (interfering) 4 different waves together, it creates that
    // messy, natural "wind-blown" look.
    float y = 0.0;
    
    // wave 1: the main direction
    y += CalculateWave(pos, vec2(1.0, 1.0), 1.0, 1.0);
    
    // wave 2: a cross-wind to break up the pattern
    y += CalculateWave(pos, vec2(0.7, -0.7), 1.1, 0.8) * 0.5; 
    
    // wave 3 & 4: smaller "choppy" waves for detail
    y += CalculateWave(pos, vec2(-1.0, 0.2), 1.3, 1.5) * 0.3; 
    y += CalculateWave(pos, vec2(0.2, -0.9), 0.9, 2.0) * 0.2;

    // applying the total height to the vertex
    pos.y += y;

    

//[Image of wave interference patterns]


    // --- ANALYTICAL NORMALS ---
    // when the water moves, the "normal" (the direction the surface faces) changes.
    // if we didn't update the normal, the lighting would stay flat and the 
    // water would look like a moving sticker.
    // we "cheat" by checking the height of two points very close by (d) 
    // and finding the slope between them.
    float d = 0.1; 
    float h1 = CalculateWave(pos + vec3(d,0,0), vec2(1,1), 1.0, 1.0); 
    float h2 = CalculateWave(pos + vec3(0,0,d), vec2(1,1), 1.0, 1.0); 
    
    // v1 and v2 are the "vectors" of the slope
    vec3 v1 = vec3(d, h1 - y, 0.0);
    vec3 v2 = vec3(0.0, h2 - y, d);
    
    // the 'cross product' gives us a vector pointing straight off that slope
    Normal = normalize(cross(v2, v1));

    

    // calculating the final positions for the screen
    FragPos = vec3(model * vec4(pos, 1.0));
    ViewPos = viewPos;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}