#include "fractal3d.h"

/* fractal3d.cu -- part of the CUDA fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

__device__ float4 juliabulb(const float3 dim_limits, const float r, const float theta, const float phi)
{
    float4 out_coords;
    out_coords.w = r * r * r * r * r * r * r * r;
    out_coords.x = dim_limits.x + out_coords.w * cos(theta) * cos(phi) + 0.353;
    out_coords.y = dim_limits.y + out_coords.w * sin(theta) * cos(phi) + 0.288;
    out_coords.z = dim_limits.z + out_coords.w * sin(phi) + 0.2;
    return out_coords;
}

__device__ float4 mandelbulb(const float3 dim_limits, const float r, const float theta, const float phi)
{
    float4 out_coords;
    out_coords.w = r * r * r * r * r * r * r * r;
    out_coords.x = dim_limits.x + out_coords.w * cos(theta) * cos(phi);
    out_coords.y = dim_limits.y + out_coords.w * sin(theta) * cos(phi);
    out_coords.z = dim_limits.z + out_coords.w * sin(phi);
    return out_coords;
}

//TODO: implement some other fractal functions here....

template <typename pixel_t, int FRACTAL_ID> 
__global__ void fractal3d_kernel (pixel_t* image,
                           const int depth_idx,
                           const int4 dimensions,
                           const int2 INT_CONSTANTS,
                           const float4 FLT_CONSTANTS)
{
    const float MIN_LIMIT = FLT_CONSTANTS.x;
    const float MAX_LIMIT = FLT_CONSTANTS.y;
    const int ORDER = INT_CONSTANTS.y;

		const int index_x = blockIdx.x * blockDim.x + threadIdx.x;    
    const int index_y = blockIdx.y * blockDim.y + threadIdx.y;

    float3 dim_limits;
    dim_limits.x = MIN_LIMIT + index_x * ((MAX_LIMIT - MIN_LIMIT) / dimensions.x);
    dim_limits.y = MIN_LIMIT + index_y * ((MAX_LIMIT - MIN_LIMIT) / dimensions.y);
    dim_limits.z = MIN_LIMIT + depth_idx * ((MAX_LIMIT - MIN_LIMIT) / dimensions.z);

    float4 coords = (float4) {0.0f, 0.0f, 0.0f, 0.0f};

    float r = 0.0f;
    float theta = 0.0f;
    float phi = 0.0f;
    pixel_t iter_num = 0;
    for (iter_num = 0; iter_num < INT_CONSTANTS.x; ++iter_num)
    {
        r = sqrt(coords.x * coords.x + coords.y * coords.y + coords.z * coords.z);
        if(r > FLT_CONSTANTS.z)
            break;

        theta = ORDER * atan2(sqrt(coords.x * coords.x + coords.y * coords.y), coords.z);
        phi   = ORDER * atan2(coords.x, coords.y);
        
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
    image[index_y * dimensions.x + index_x] = max(0, iter_num-1);
}

//--------------------------------------------------------------------------------------------------------------------------------

template <typename pixel_t, int FRACTAL_ID> __host__ 
void run_fractalgen(pixel_t* dev_image, int depth_idx, const int4 dimensions, const int2 constants, const float4 flt_constants)
{
  static constexpr int blockdim = 16;
  //want to process a frame per kernel invocation -- frames will be something e.g. [128 x 128], [512 x 512], [1024 x 1024], etc. 
  dim3 block_dim (blockdim, blockdim);
  dim3 grid_dim  (static_cast<int>(std::ceil(dimensions.x / static_cast<float>(blockdim))), static_cast<int>(std::ceil(dimensions.y / static_cast<float>(blockdim))));

  fractal3d_kernel<pixel_t, FRACTAL_ID><<<grid_dim, block_dim>>> (dev_image, depth_idx, dimensions, constants, flt_constants);
}

template __host__ void run_fractalgen <unsigned char, 0> (unsigned char*, int, const int4, const int2, const float4);
template __host__ void run_fractalgen <unsigned char, 1> (unsigned char*, int, const int4, const int2, const float4);

