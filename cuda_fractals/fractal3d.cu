#include <stdio.h>
#include <assert.h>
#include <cuda_runtime.h>

__device__ float4 juliabulb(const float3 dim_limits, const float r, const float theta, const float phi)
{
    float4 out_coords;
    out_coords.s3 = r * r * r * r * r * r * r * r;
    out_coords.s0 = dim_limits.s0 + out_coords.s3 * cos(theta) * cos(phi) + 0.353;
    out_coords.s1 = dim_limits.s1 + out_coords.s3 * sin(theta) * cos(phi) + 0.288;
    out_coords.s2 = dim_limits.s2 + out_coords.s3 * sin(phi) + 0.2;
    return out_coords;
}

__device__ float4 mandelbulb(const float3 dim_limits, const float r, const float theta, const float phi)
{
    float4 out_coords;
    out_coords.s3 = r * r * r * r * r * r * r * r;
    out_coords.s0 = dim_limits.s0 + out_coords.s3 * cos(theta) * cos(phi);
    out_coords.s1 = dim_limits.s1 + out_coords.s3 * sin(theta) * cos(phi);
    out_coords.s2 = dim_limits.s2 + out_coords.s3 * sin(phi);
    return out_coords;
}

template <int FRACTAL_ID> 
__global__ void fractal3d
         (int* restrict image,
          const int depth_idx,
          const int3 dimensions,
          const int2 INT_CONSTANTS,
          const float3 FLT_CONSTANTS)
{
    const float MIN_LIMIT = FLT_CONSTANTS.s0;
    const float MAX_LIMIT = FLT_CONSTANTS.s1;
    const int ORDER = INT_CONSTANTS.s1;

    float3 dim_limits;
    dim_limits.s0 = MIN_LIMIT + get_global_id(0) * ((MAX_LIMIT - MIN_LIMIT) / dimensions.s0);
    dim_limits.s1 = MIN_LIMIT + get_global_id(1) * ((MAX_LIMIT - MIN_LIMIT) / dimensions.s1);
    dim_limits.s2 = MIN_LIMIT + depth_idx * ((MAX_LIMIT - MIN_LIMIT) / dimensions.s2);

    float4 coords;
    coords.s0 = 0.0f;
    coords.s1 = 0.0f;
    coords.s2 = 0.0f;
    coords.s3 = 0.0f;

    float r = 0.0f;
    float theta = 0.0f;
    float phi = 0.0f;
    int iter_num = 0;
    int i = 0;
    for (iter_num = 0; iter_num < INT_CONSTANTS.s0; ++iter_num)
    {
        r = sqrt(coords.s0 * coords.s0 + coords.s1 * coords.s1 + coords.s2 * coords.s2);
        if(r > FLT_CONSTANTS.s2)
            break;

        theta = ORDER * atan2(sqrt(coords.s0 * coords.s0 + coords.s1 * coords.s1), coords.s2);
        phi =   ORDER * atan2(coords.s0, coords.s1);
        
        //use the given fractal type
        switch(FRACTAL_ID)
        {
          case MANDELBROT:
          {
  				  coords = mandelbulb(dim_limits, r, theta, phi);  
            break;
          }
          case JULIA:
          {
  				  coords = juliabulb(dim_limits, r, theta, phi);  
            break;
          }        
        }
    }
    
    image[threadIdx.x + blockIdx.x*blockDim.x] = max(0, iter_num-1);
}                      




