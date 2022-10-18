//------------------------------
//- main.hlsl
//------------------------------

// Includes
#include "include.hlsli"

cbuffer constants : register(b0)
{    
	// Projection view matrix
    float4x4 projInverse;
    float4x4 viewInverse;
    float3 camPos;
    float padding1;

	// Screen dimensions
    int screenWidth;
    int screenHeight;
    
    // Animation and quality
    int animated;
    int quality;

	// Time since execution start
    float time;

	// Colours
    float3 colour1;
    float3 colour2;
    float padding2;
}; 

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD0)
{
    PSInput result;

    result.position = float4(position.xyz, 1.0f);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // Scene constants
    float3 light1 = float3(10.0f, 10.0f, -10.0f);
    float3 light2 = float3(-10.0f, 10.0f, -10.0f);
    float3 light3 = float3(0.0f, 0.0f, 10.0f);
        
    // Fractal parameters
    FractalOptions params;
    params.maxIters = 25;
    params.escape = 256.0f;
    
    if (animated == 1)
    {
        // Animate power from 5-9
        float interval = frac(time * 0.00025f);
        params.power = 7.0f + (sin(interval * 6.28f) * 2.0f);
    }
    else
    {
        params.power = 8.0f;
    }
        
    // Get UV coordinates. input.position.xy stores PIXEL coords
    float2 uv = input.position.xy / float2(screenWidth, screenHeight);
    
    // Change UV to -1 to 1 range
    uv = (uv - float2(0.5f, 0.5f)) * 2.0f;
    // Flip V
    uv.y *= -1;
                
    // Initialize variables and create camera ray
    Ray ray = CreateCamRay(uv, projInverse, viewInverse, camPos);
    
    float minDist;
    int maxIters;
    // Quality 
    switch (quality)
    {
        case 0:
            minDist = 0.001f;
            maxIters = 80;
            break;
        case 1:
            minDist = 0.0005f;
            maxIters = 128;
            break;
        case 2:
            minDist = 0.0001f;
            maxIters = 160;
            break;
    }
        
    float totalDistance = 0; // Total distance travelled
    float distFromScene = DistToScene(ray.pos, params); // The distance we can safely move the ray without collision
    float closestDistance = 1.#INF;
    
    // Default colour (Vignette background)
    float3 colour = ((colour1 + colour2) / 2.0f / 255.0f) - (float3(length(uv), length(uv), length(uv)) / 2.0f);
    
    float lenZ = 0;
    
    // Raymarching
    for (int iter = 0; iter < maxIters; iter++)
    {
        ray.pos += ray.dir * distFromScene; // Move the ray forward as far as we are sure no collisions occur
        totalDistance += distFromScene;
        distFromScene = DistToScene(ray.pos, params, lenZ); // Update distance to scene
        
        if (distFromScene < closestDistance)
        {
            closestDistance = distFromScene;
        }
        
        // Out of Mandelbulb range
        if (length(ray.pos) > 2.5f)
            break;
        
        if (distFromScene < minDist)
        {
            // Hit mandelbulb, shade
            colour = (colour1 + colour2) / 255.0f / 20.0f; // Ambient
            
            // Normal estimate
            float3 normal = NormalEstimate(ray.pos, params);
            
            // Shadows
            float3 diffuse1 = SoftShadow(ray.pos + 0.01f * normal, normalize(light1 - ray.pos), minDist, 4.0f, 3.0f, params) // Light 1 
            * float3(0.809f, 0.878f, 1.0f); // Sky blue
            
            float3 diffuse2 = SoftShadow(ray.pos + 0.01f * normal, normalize(light2 - ray.pos), minDist, 4.0f, 3.0f, params) // Light 2 
            * float3(1.0f, 0.945f, 0.878f); // Lightbulb orange
            
            float3 diffuse3 = SoftShadow(ray.pos + 0.01f * normal, normalize(light3 - ray.pos), minDist, 4.0f, 3.0f, params) // Light 3
            * float3(0.796f, 0.765f, 0.890f); // Purple
            
            colour += saturate(diffuse1 + diffuse2 + diffuse3) * (lerp(colour1, colour2, saturate(lenZ / 10.0f)) / 255.0f);
            
            break;
        }
    }
                
    colour = saturate(colour); // Clamp to 0-1 range

    return float4(colour, 1.0f);
}