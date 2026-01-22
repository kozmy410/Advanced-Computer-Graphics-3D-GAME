#version 330 core
out vec4 FragColor; // the final color of the pixel on the screen

// input data coming from the vertex shader (position, texture coords, normals, etc.)
in VS_OUT {
    vec3 FragPos;           // where this pixel is in the 3d world
    vec2 TexCoords;         // uv coordinates for textures
    vec3 TangentViewPos;    // camera position adjusted for bump mapping
    vec3 TangentFragPos;    // pixel position adjusted for bump mapping
    vec3 Normal;            // surface direction
    vec3 Tangent;           // texture direction x
    vec3 Bitangent;         // texture direction y
    vec4 FragPosLightSpace; // pixel position from the sun's perspective (for shadows)
} fs_in;

// --- TEXTURE SAMPLERS ---
// these are the images we loaded in c++ and sent to the gpu
uniform sampler2D texture_diffuse1;   // base color (albedo)
uniform sampler2D texture_normal1;    // bump details (purple map)
uniform sampler2D texture_height1;    // depth details (displacement)
uniform sampler2D texture_metallic1;  // which parts are metal
uniform sampler2D texture_roughness1; // which parts are shiny vs matte
uniform sampler2D texture_ao1;        // ambient occlusion (fake shadows in cracks)
uniform sampler2D shadowMap;          // the shadow map generated in the first pass
uniform samplerCube skybox;           // the skybox for reflections

// --- FLAGS ---
// booleans to tell the shader which textures actually exist
uniform bool hasTexture;
uniform bool hasNormalMap;
uniform bool hasDispMap;
uniform bool hasMetallicMap;
uniform bool hasRoughnessMap;
uniform bool hasAOMap;

// --- LIGHTING DATA ---
uniform vec3 lightPos[64];    // positions of all lights
uniform vec3 lightColors[64]; // colors/intensity of all lights
uniform int nrLights;         // how many lights are currently active
uniform vec3 viewPos;         // where the camera is looking from

// --- PER-OBJECT UNIFORMS ---
// default values if no texture map is provided
uniform float u_Metallic; 
uniform float u_Roughness;
uniform float u_AO;

const float PI = 3.14159265359;

// ==========================================================================
// 1. SHADOW CALCULATION
// ==========================================================================
// this function checks if a pixel is hidden from the sun by another object
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // 1. convert position to range [0,1] so we can read the shadow map texture
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // if the point is too far away, assume it's not in shadow
    if(projCoords.z > 1.0) return 0.0;
    
    // 2. calculate bias to prevent "shadow acne" (weird striped patterns on surfaces)
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    // 3. pcf (percentage-closer filtering)
    // instead of checking just one pixel, we check the surrounding pixels in the shadow map
    // and average them. this creates "soft shadows" instead of jagged blocky ones.
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0; // average the result
    return shadow;
}

// ==========================================================================
// 2. PARALLAX MAPPING
// ==========================================================================
// this creates a 3d illusion on flat surfaces (like bricks popping out)
// it shifts the texture coordinates based on the viewing angle and height map.
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) { 
    // determines how many "layers" of depth to check. steep angles get more layers for accuracy.
    const float minLayers = 32.0; 
    const float maxLayers = 128.0; 
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir))); 
    float layerDepth = 1.0 / numLayers; 
    float currentLayerDepth = 0.0; 
    
    // how deep the effect looks (adjustable)
    float height_scale = 0.001; 
    vec2 P = viewDir.xy / max(viewDir.z, 0.1) * height_scale; 
    vec2 deltaTexCoords = P / numLayers; 
    
    vec2 currentTexCoords = texCoords; 
    float currentDepthMapValue = texture(texture_height1, currentTexCoords).r; 
    
    // loop until we find the "collision" point where the view ray hits the fake height geometry
    int maxIterations = int(numLayers); 
    int iterations = 0; 
    while(currentLayerDepth < currentDepthMapValue && iterations < maxIterations) { 
        currentTexCoords -= deltaTexCoords; 
        currentDepthMapValue = texture(texture_height1, currentTexCoords).r; 
        currentLayerDepth += layerDepth; 
        iterations++; 
    } 
    
    // interpolation to smooth out the steps between layers (parallax occlusion mapping)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords; 
    float afterDepth = currentDepthMapValue - currentLayerDepth; 
    float beforeDepth = texture(texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth; 
    float weight = afterDepth / (afterDepth - beforeDepth); 
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight); 
}

// ==========================================================================
// 3. PBR MATH HELPER FUNCTIONS
// ==========================================================================
// physically based rendering (pbr) uses complex math to simulate how light bounces off matter.
// you don't need to fully understand the math, just that it balances reflections and diffusion.

// calculates how many "micro-facets" of the surface are aligned with the light
float DistributionGGX(vec3 N, vec3 H, float roughness) { 
    float a = roughness*roughness; float a2 = a*a; float NdotH = max(dot(N, H), 0.0); float NdotH2 = NdotH*NdotH; float nom = a2; float denom = (NdotH2 * (a2 - 1.0) + 1.0); denom = PI * denom * denom; return nom / max(denom, 0.0000001); 
}
// calculates how much light gets blocked by the surface's own roughness (self-shadowing)
float GeometrySchlickGGX(float NdotV, float roughness) { 
    float r = (roughness + 1.0); float k = (r*r) / 8.0; float nom = NdotV; float denom = NdotV * (1.0 - k) + k; return nom / max(denom, 0.0000001); 
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) { 
    float NdotV = max(dot(N, V), 0.0); float NdotL = max(dot(N, L), 0.0); float ggx2 = GeometrySchlickGGX(NdotV, roughness); float ggx1 = GeometrySchlickGGX(NdotL, roughness); return ggx1 * ggx2; 
}
// calculates how reflective a surface becomes at glancing angles (fresnel effect)
vec3 fresnelSchlick(float cosTheta, vec3 F0) { 
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); 
}

// ==========================================================================
// 4. MAIN
// ==========================================================================
void main()
{           
    // calculating direction from pixel to eye
    vec3 tangentViewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    // 1. parallax mapping logic
    // if we have a height map, we shift texture coords to create depth
    if (hasDispMap) {
        vec2 parallaxCoords = ParallaxMapping(fs_in.TexCoords, tangentViewDir);
        // creating a "clipping" check so the texture doesn't tile weirdly at the edges
        if(parallaxCoords.x >= 0.0 && parallaxCoords.x <= 1.0 && parallaxCoords.y >= 0.0 && parallaxCoords.y <= 1.0)
            texCoords = parallaxCoords;
    }
    
    // --- loading the base color ---
    vec4 diffuseSample = hasTexture ? texture(texture_diffuse1, texCoords) : vec4(1.0);
    
    // converting color from srgb (monitor colors) to linear space (math colors) for accurate lighting
    vec3 albedo = pow(diffuseSample.rgb, vec3(2.2)); 
    
    // extracting transparency (alpha)
    float alpha = diffuseSample.a;

    // --- alpha cutout ---
    // if a pixel is almost transparent (like the gap in a chain link fence), 
    // we discard it entirely so we can see through it.
    if (alpha < 0.1) discard; 

    // loading material properties (either from textures or default sliders)
    float metallic = hasMetallicMap ? texture(texture_metallic1, texCoords).r : u_Metallic;
    float roughness = hasRoughnessMap ? texture(texture_roughness1, texCoords).r : u_Roughness;
    float ao = hasAOMap ? texture(texture_ao1, texCoords).r : u_AO;

    // 3. normal mapping logic
    // this tricks the light into thinking a flat surface is bumpy
    vec3 N_world;
    if (hasNormalMap) {
        vec3 normalMapVal = texture(texture_normal1, texCoords).rgb;
        normalMapVal = normalize(normalMapVal * 2.0 - 1.0); // converting [0,1] color range to [-1,1] vector range
        
        // constructing the tbn matrix to transform the normal from tangent space to world space
        vec3 T = normalize(fs_in.Tangent);
        vec3 B = normalize(fs_in.Bitangent);
        vec3 N = normalize(fs_in.Normal);
        mat3 TBN = mat3(T, B, N);
        N_world = normalize(TBN * normalMapVal);
    } else {
        // if no normal map, just use the flat geometry normal
        N_world = normalize(fs_in.Normal);
    }

    // 4. shadow calculation
    // we assume the first light (index 0) is the "sun" that casts shadows
    vec3 sunDir = normalize(lightPos[0] - fs_in.FragPos); 
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, N_world, sunDir);

    // 5. pbr lighting loop
    vec3 V = normalize(viewPos - fs_in.FragPos); // vector pointing to camera
    
    // f0 represents how reflective the material is at a straight-on angle.
    // non-metals (dielectrics) are usually 0.04. metals use their albedo color.
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0); // accumulative light output

    // looping through every light source
    for(int i = 0; i < nrLights; ++i) {
        
        float distance = length(lightPos[i] - fs_in.FragPos);
        float maxRadius = sqrt(length(lightColors[i]) / 0.1); // approximation of light radius

        // optimization: if light is too far, don't calculate complex math
        if (distance < maxRadius) {
            vec3 L = normalize(lightPos[i] - fs_in.FragPos); // direction to light
            vec3 H = normalize(V + L); // halfway vector (essential for specular highlights)
            
            // calculating light falloff (inverse square law) - lights get dimmer with distance
            float attenuation = 1.0 / (distance * distance); 
            float factor = distance / maxRadius;
            float window = clamp(1.0 - factor * factor * factor * factor, 0.0, 1.0);
            attenuation *= window * window; // smooth falloff to zero
            
            vec3 radiance = lightColors[i] * attenuation; 

            // calculating the three core components of pbr:
            // 1. D = distribution (specular shape/size)
            // 2. G = geometry (self-shadowing of micro-bumps)
            // 3. F = fresnel (reflectivity at angles)
            float NDF = DistributionGGX(N_world, H, roughness);   
            float G   = GeometrySmith(N_world, V, L, roughness);      
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
            
            // combining the components into the specular (shiny) part
            vec3 numerator    = NDF * G * F; 
            float denominator = 4.0 * max(dot(N_world, V), 0.0) * max(dot(N_world, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;
            
            // energy conservation: light that reflects (specular) cannot also diffuse (color)
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic; // metals have no diffuse color, they are purely reflective
            
            float NdotL = max(dot(N_world, L), 0.0); // how direct is the light hitting the surface?
            
            // adding this light's contribution to the total
            vec3 perLightResult = (kD * albedo / PI + specular) * radiance * NdotL;

            // applying shadow only to the first light (sun)
            if (i == 0) Lo += (1.0 - shadow) * perLightResult;
            else Lo += perLightResult;
        }
    }   

    // 6. environment reflection (ibl approximation)
    // reflecting the skybox on shiny surfaces
    vec3 R = reflect(-V, N_world);
    vec3 skyReflection = texture(skybox, R).rgb;
    // mixing sky reflection based on roughness (rougher surfaces reflect less clear sky)
    vec3 reflection = skyReflection * metallic * (1.0 - roughness);

    // 7. combining everything
    vec3 ambient = vec3(0.03) * albedo * ao; // faint base light so shadows aren't pitch black
    vec3 color = ambient + Lo + reflection;

    // 8. tone mapping (reinhard)
    // compressing high dynamic range (hdr) colors (brightness > 1.0) down to [0,1] range for the monitor
    color = color / (color + vec3(1.0)); 
    
    // 9. gamma correction
    // converting linear colors back to srgb for display
    color = pow(color, vec3(1.0/2.2)); 

    // final output
    FragColor = vec4(color, alpha);
}