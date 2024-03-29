/* fractal3d.h -- part of the CUDA fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef FRACTAL_GEN_CUDA_FRACTALS_FRACTAL3D_CUH
#define FRACTAL_GEN_CUDA_FRACTALS_FRACTAL3D_CUH

#include <stdio.h>
#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>

#define MANDELBROT 0
#define JULIA 1

template <typename pixel_t, int FRACTAL_ID> __host__ 
void run_fractalgen(pixel_t* dev_image, int depth_idx, const int4 dimensions, const int2 constants, const float4 flt_constants);

#endif
