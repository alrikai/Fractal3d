/* fractalgen3d.hpp -- part of the CUDA fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef CUDA_FRACTALS_3D_HPP
#define CUDA_FRACTALS_3D_HPP

#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <tuple>
#include <complex>
#include <memory>
#include <array>
#include <chrono>
#include <stdexcept>

#include <cuda.h>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>

//#include "../cpu_fractal.hpp"
//#include "util/ocl_helpers.hpp"
#include "fractal3d.h"

namespace fractal_helpers
{
/*
struct fractal_options
{
  static const std::vector<std::string>& get_ids()
  {
    static const std::vector <std::string> ids 
    {
      "mandelbrot",
      "julia"
    };
    return ids;
  }

  static std::string get_kernel_id(const std::string& id)
  {
    static const std::map <std::string, std::string> ocl_ids
    {
      {"mandelbrot", "MANDELBROT"},
      {"julia", "JULIA"}
    };

    auto id_it = ocl_ids.find(id);
    if(id_it != ocl_ids.end())
      return id_it->second;
    else
      throw std::runtime_error("INVALID Fractal Name -- " + id);
  }
};
*/


} //namespace fractal_helpers

inline void cuda_error_check(cudaError_t err_id)
{
  if(err_id != cudaSuccess)
  {
    std::stringstream cu_ss;
    cu_ss << __FILE__ << " at: " << __LINE__;
    std::string error_msg;
    cu_ss >> error_msg;

    error_msg += cudaGetErrorName(err_id);
    throw std::runtime_error(error_msg);
  }
}


template <typename data_t>
void run_cuda_fractal(std::vector<data_t>& h_image_stack, const fractal_params& params)
{
  std::cout << "Using CUDA" << std::endl;

  const size_t imageslice_sz = params.imheight * params.imwidth * sizeof(data_t);
  data_t* dev_image;
  cudaError_t cu_error_id = cudaMalloc((void**)&dev_image, imageslice_sz);
  cuda_error_check (cu_error_id);
 
  auto start = std::chrono::high_resolution_clock::now();

  const int2 constants = {static_cast<int>(params.MAX_ITER), params.ORDER};
  const int4 dimensions = {params.imheight, params.imwidth, params.imdepth, 0};
  const float4 flt_constants = {params.MIN_LIMIT, params.MAX_LIMIT, params.BOUNDARY_VAL, 0.0f};

  for (int depth_idx = 0; depth_idx < params.imdepth; ++depth_idx)
  {
    cu_error_id = cudaMemset(dev_image, 0, imageslice_sz);
    cuda_error_check (cu_error_id);

    //e.g. MANDELBROT --> 0, JULIA --> 1, ETC... TODO: define this mapping...
    static constexpr int FRACTAL_ID = JULIA; //MANDELBROT;
    run_fractalgen<data_t, FRACTAL_ID> (dev_image, depth_idx, dimensions, constants, flt_constants);

    //------------------------------------------------------
    //NOTE: shouldnt be necessary since we're only using 1 stream
    cu_error_id = cudaDeviceSynchronize();
    cuda_error_check (cu_error_id);
    //------------------------------------------------------

    const int h_image_stack_offset = params.imheight * params.imwidth * depth_idx;
    auto host_slice_image = &h_image_stack[h_image_stack_offset];
    cu_error_id = cudaMemcpy(host_slice_image, dev_image, imageslice_sz, cudaMemcpyDeviceToHost); 
    cuda_error_check (cu_error_id);
  
    bool verbose_run = false;
    if(verbose_run)
    {
      auto slice_sum = std::accumulate(&h_image_stack[h_image_stack_offset], &h_image_stack[h_image_stack_offset] + imageslice_sz, 0);
      std::cout << "slice " << depth_idx << " sum: " << slice_sum << std::endl;
    }
  }

  //---------------------------------------------------------------

  cu_error_id = cudaFree(dev_image);
  cuda_error_check (cu_error_id);
//  cudaDeviceReset(); 

/*
    auto fracids = fractal_helpers::fractal_options::get_ids();
    std::string fractal_id_list = "";
    std::for_each(fracids.begin(), fracids.end(), [&fractal_id_list](const std::string& id)
    {
      fractal_id_list += (" -- " + fractal_helpers::fractal_options::get_ocl_id(id));
    });

    std::cout << "fractal ID list: " << fractal_id_list << std::endl;
    const std::string ocl_fractal_id = fractal_helpers::fractal_options::get_ocl_id(params.fractal_name);
    const std::string cl_opts {"-DFRACTALID=" + ocl_fractal_id};
    ocl_error_num = clBuildProgram(ocl_program, 0, 0, cl_opts.c_str(), nullptr, nullptr);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ PROGRAM BUILD -- " << ocl_error_num << std::endl;
*/

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    std::cout << "Fractal Generation Time: " << duration.count() << " ms" << std::endl;

}

#endif
