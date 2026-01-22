#version 330 core
// taking in raw data from the 3d model file
layout (location = 0) in vec3 aPos;      // local position of the vertex
layout (location = 1) in vec3 aNormal;   // which way the surface faces
layout (location = 2) in vec2 aTexCoord; // uv coordinates for the texture
layout (location = 3) in vec3 aTangent;  // used to calculate bump mapping direction

// things we are sending over to the fragment shader
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 Normal;     
    vec3 Tangent;    
    vec3 Bitangent; 
    vec4 FragPosLightSpace; 
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix; // used for shadows

uniform vec3 viewPos; // where the camera is

void main()
{
    // 1. world position: moving the point from "model space" to the actual game world
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoord;
    
    // 2. normal matrix: this ensures our surface directions still point the right way 
    // even if the object is rotated or scaled weirdly.
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    
    // 3. calculate TBN (tangent, bitangent, normal) in world space
    // these three vectors form a mini coordinate system on the surface of the object.
    // this is what makes 2d normal maps look like 3d bumps.
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    
    // Gram-Schmidt process: this "cleans up" the math. 
    // it makes sure the tangent vector is perfectly at a 90-degree angle to the normal.
    T = normalize(T - dot(T, N) * N);
    
    // the bitangent is simply the third direction, calculated by crossing the other two.
    vec3 B = cross(N, T);
    
    // sending these vectors out so the fragment shader can calculate lighting
    vs_out.Normal = N;
    vs_out.Tangent = T;
    vs_out.Bitangent = B;

    

    // 4. parallax mapping data
    // parallax mapping needs to know where the camera and the pixel are relative 
    // to the "surface's own perspective" (tangent space).
    mat3 TBN = transpose(mat3(T, B, N)); 
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

    // 5. shadow data
    // calculating where this point is from the perspective of the sun/light
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    
    
        
    // finally, tell opengl where to draw this point on the 2d screen
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}