//------------------------------
//- include.hlsli
//------------------------------

// Camera ray
struct Ray
{
    float3 pos;
    float3 dir;
};

// Fractal options
struct FractalOptions
{
    int maxIters;
    float escape;
    float power;
};

// https://sibaku.github.io/computer-graphics/2017/01/10/Camera-Ray-Generation.html
Ray CreateCamRay(float2 uv, float4x4 projInverse, float4x4 viewInverse, float3 camPos)
{
    Ray ray;
    
    float4 ndsh = float4(uv, -1.0f, 1.0f);
    float4 dirEye = mul(projInverse, ndsh);
    dirEye.w = 0;
    
    float3 dirWorld = mul(viewInverse, dirEye).xyz;
    
    ray.pos = camPos;
    ray.dir = normalize(dirWorld);
    
    return ray;
}

// https://iquilezles.org/articles/mandelbulb/
float DistToScene(float3 pos, FractalOptions params)
{
    float3 z = pos;
    z.xyz = z.xzy;
    float m = dot(pos, pos);
    
    float dz = 1.0;
    float r = 0.0;
    for (int i = 0; i < params.maxIters; i++)
    {
        dz = 8.0 * pow(m, 3.5) * dz + 1.0;
      
        // z = z^8+c
        float r = length(z);
        float b = params.power * acos(z.y / r);
        float a = params.power * atan2(z.x, z.z);
        z = pos.xzy + pow(r, 8.0) * float3(sin(b) * sin(a), cos(b), sin(b) * cos(a));
        
        m = dot(z, z);
        if (m > params.escape)
            break;
    }
    
    return 0.25 * log(m) * sqrt(m) / dz;
}

// Same as above, but return the highest value of length(z) before escape
float DistToScene(float3 pos, FractalOptions params, out float lenZ)
{
    float3 z = pos;
    z.xyz = z.xzy;
    float m = dot(pos, pos);
    lenZ = length(z);
    
    float dz = 1.0;
    float r = 0.0;
    for (int i = 0; i < params.maxIters; i++)
    {   
        dz = 8.0f * pow(m, 3.5f) * dz + 1.0f;
      
        // z = z^8+c
        float r = length(z);
        float b = params.power * acos(z.y / r);
        float a = params.power * atan2(z.x, z.z);
        z = pos.xzy + pow(r, 8.0f) * float3(sin(b) * sin(a), cos(b), sin(b) * cos(a));
        
        m = dot(z, z);
        
        if (m > params.escape)
            break;
        
        if (length(z) > lenZ)
            lenZ = length(z);
    }
        
    return 0.25 * log(m) * sqrt(m) / dz;
}

// https://iquilezles.org/articles/rmshadows/
float SoftShadow(in float3 hit, in float3 lightDir, float mint, float maxt, float k, FractalOptions params)
{
    // Passing nearby an object but missing will result in a penumbra
    // Use the closest distance before hit to create a soft shadow
    float res = 1.0;
    float t = 0.0f;
    for (int i = 0; i < params.maxIters; i++)
    {
        // Out of Mandelbulb range
        if (length(hit + lightDir * t) > 3.0f)
            break;
        
        float h = DistToScene(hit + lightDir * t, params);
        if (h < 0.001f)
        {
            // Hit an object, in complete shadow
            return 0.0;
        }
        res = min(res, k * h / t);
        t += h;
    }
    return res;
}

// Normal estimate which compares distance with respect to dy,dx,dz to get normal
float3 NormalEstimate(float3 p, FractalOptions params)
{
    double EPS = 0.0001f;
    double xPl = DistToScene(float3(p.x + EPS, p.y, p.z), params);
    double xMi = DistToScene(float3(p.x - EPS, p.y, p.z), params);
    double yPl = DistToScene(float3(p.x, p.y + EPS, p.z), params);
    double yMi = DistToScene(float3(p.x, p.y - EPS, p.z), params);
    double zPl = DistToScene(float3(p.x, p.y, p.z + EPS), params);
    double zMi = DistToScene(float3(p.x, p.y, p.z - EPS), params);
    double xDiff = xPl - xMi;
    double yDiff = yPl - yMi;
    double zDiff = zPl - zMi;
    return normalize(float3(xDiff, yDiff, zDiff));
}