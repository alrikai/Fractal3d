#ifndef FRACTAL_GEN_CUDA_FRACTALS_FRACTAL3D_CUH
#define FRACTAL_GEN_CUDA_FRACTALS_FRACTAL3D_CUH

#include <stdio.h>
#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>

#define MANDELBROT 0
#define JULIA 1

template <int FRACTAL_ID> __host__ 
void run_fractalgen(int* dev_image, int depth_idx, const int3 dimensions, const int2 constants, const float3 flt_constants);

#endif
