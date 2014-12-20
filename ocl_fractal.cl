__kernel void fractal2d
         (__global uint* image,
          const float4 limits,
          const float2 factor,
          const float BOUNDARY_VAL,
          const int imwidth,
          const int MAX_ITER)
{
    const int tid = get_global_id(0);
    int row_idx = tid / imwidth;
    int col_idx = tid % imwidth;

    float Zfactor_imag = limits.s3 - row_idx * factor.s1;
    float Zfactor_real = limits.s0 + col_idx * factor.s0;
    
    float Zi = Zfactor_imag;
    float Zr =Zfactor_real;
  
    int iter_num = 0;
    for (iter_num = 0; iter_num < MAX_ITER; ++iter_num)
    {
        if(sqrt(Zr * Zr + Zi * Zi) > BOUNDARY_VAL)
            break;

        float real_tmp = Zr * Zr - Zi * Zi;
        float imag_tmp = 2 * Zr * Zi;
        Zr = real_tmp + Zfactor_real;
        Zi = imag_tmp + Zfactor_imag;
    }
    image[tid] = iter_num;
}

float4 juliabulb(const float4 coords, const float3 dim_limits, const float r, const float theta, const float phi)
{
    float4 out_coords;
    out_coords.s3 = r * r * r * r * r * r * r * r;
    out_coords.s0 = dim_limits.s0 + coords.s3 * cos(theta) * cos(phi) + 0.353;
    out_coords.s1 = dim_limits.s1 + coords.s3 * sin(theta) * cos(phi) + 0.288;
    out_coords.s2 = dim_limits.s2 + coords.s3 * sin(phi) + 0.2;
}

float4 mandelbulb(const float4 coords, const float3 dim_limits, const float r, const float theta, const float phi)
{
    float4 out_coords;
    out_coords.s3 = r * r * r * r * r * r * r * r;
    out_coords.s0 = dim_limits.s0 + coords.s3 * cos(theta) * cos(phi);
    out_coords.s1 = dim_limits.s1 + coords.s3 * sin(theta) * cos(phi);
    out_coords.s2 = dim_limits.s2 + coords.s3 * sin(phi);
}

__kernel void fractal3d
         (__global int* restrict image,
          const int depth_idx,
          const int3 dimensions,
          const int2 INT_CONSTANTS,
          const float3 FLT_CONSTANTS)
{
    const float MAX_LIMIT = FLT_CONSTANTS.s1;
    const float MIN_LIMIT = FLT_CONSTANTS.s0;
    const int ORDER = INT_CONSTANTS.s1;

    float3 dim_limits;
    dim_limits.s0 = MIN_LIMIT + get_global_id(0) * ((MAX_LIMIT - MIN_LIMIT) / dimensions.s0);
    dim_limits.s1 = MIN_LIMIT + get_global_id(1) * ((MAX_LIMIT - MIN_LIMIT) / dimensions.s1);
    dim_limits.s2 = MIN_LIMIT + depth_idx * ((MAX_LIMIT - MIN_LIMIT) / dimensions.s2);

    float4 coords; 
    coords.s0 = 0; 
    coords.s1 = 0; 
    coords.s2 = 0; 
    coords.s3 = 0;

    float r = 0;
    float theta = 0;
    float phi = 0;
    int iter_num = 0;
    int i = 0;
    for (iter_num = 0; iter_num < INT_CONSTANTS.s0; ++iter_num)
    {
        r = sqrt(coords.s0 * coords.s0 + coords.s1 * coords.s1 + coords.s2 * coords.s2);
        if(r > FLT_CONSTANTS.s2)
            break;

        theta = ORDER * atan2(sqrt(coords.s0 * coords.s0 + coords.s1 * coords.s1), coords.s2);
        phi =   ORDER * atan2(coords.s0, coords.s1);

        coords = mandelbulb(coords, dim_limits, r, theta, phi);  
    }
    image[get_global_id(0) * dimensions.s1 + get_global_id(1)] = max(0, iter_num-1);
}                      


