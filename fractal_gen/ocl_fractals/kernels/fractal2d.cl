// fractal2d.cl -- part of the OpenCL fractal3d implementation 
//
// Copyright (C) 2015 Alrik Firl 
//
// This software may be modified and distributed under the terms
// of the MIT license.  See the LICENSE file for details.
//




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
    float Zr = Zfactor_real;
  
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


