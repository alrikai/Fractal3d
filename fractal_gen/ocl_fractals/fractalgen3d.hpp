#ifndef OCL_FRACTALS_3D_HPP
#define OCL_FRACTALS_3D_HPP

#include <CL/cl.hpp>
#include <opencv2/opencv.hpp>

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

#include "../cpu_fractal.hpp"
#include "util/ocl_helpers.hpp"

namespace fractal_helpers
{

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

  static std::string get_ocl_id(const std::string& id)
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

} //namespace fractal_helpers


template <typename data_t>
void run_ocl_fractal(std::vector<data_t>& h_image_stack, const fractal_params& params)
{
  bool verbose_run = false;
	using cldata_t = cl_uchar;

    std::string target_platform_id {"NVIDIA"};
    //get ONE GPU device on the target platform 
    const int num_gpu = 1;
 
    cl_uint ocl_platform;
    bool platform_present;
    cl_platform_id ocl_platform_id;

    std::cout << "Finding platform " << target_platform_id << std::endl;
    std::tie(ocl_platform, ocl_platform_id, platform_present) = ocl_helpers::get_platform_id(target_platform_id);

    if(platform_present)
        std::cout << target_platform_id << " platform at index " << ocl_platform << std::endl;

    //5 things for every opencl host-side program:
    //1. cl_device_id
    //2. cl_command_queue
    //3. cl_context
    //4. cl_program
    //5. cl_kernel
    cl_device_id device_id;
    clGetDeviceIDs(ocl_platform_id, CL_DEVICE_TYPE_GPU, num_gpu, &device_id, nullptr);
    std::cout << "GPU device id: " << device_id << std::endl; 
    
    cl_int ocl_error_num;
    //create an opencl context
    cl_context ocl_context = clCreateContext(nullptr, num_gpu, &device_id, nullptr, nullptr, nullptr); 
    // Create a command queue for the device in the context
    cl_command_queue ocl_command_queue = clCreateCommandQueue(ocl_context, device_id, 0, nullptr);

    //NOTE: should I use boost::filepath for the ocl files?
    const std::string kernel_fpath = ocl_helpers::get_kernelpath();
    const std::string file_name { kernel_fpath + "fractal3d.cl" }; 
    std::string program_source;
    ocl_helpers::load_kernel_file(file_name, program_source);
		
    //NOTE: might be able to do some error recovery instead (e.g. switch to CPU fractals)
    assert(!program_source.empty());

    //create + compile the opencl program
    auto kernel_source_code = program_source.c_str();
    cl_program ocl_program = clCreateProgramWithSource(ocl_context, 1, (const char**) &kernel_source_code, 0, &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ PROGRAM CREATION -- " << ocl_error_num << std::endl;

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

    if (ocl_error_num == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(ocl_program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        // Allocate memory for the log
        char *log = (char *) malloc(log_size);
        // Get the log
        clGetProgramBuildInfo(ocl_program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        // Print the log
        printf("%s\n", log);

        return;
    }

    // Create kernel instance
    const std::string fractal_kernel_name = "fractal3d";
    cl_kernel ocl_kernel = clCreateKernel(ocl_program, fractal_kernel_name.c_str(), &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ KERNEL CREATION -- " << ocl_error_num  << " -- kernel name: " << fractal_kernel_name << std::endl;
  
    cl_mem dev_image;
    dev_image = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, params.imheight * params.imwidth * sizeof(cldata_t), nullptr, 0);
   
    const cl_uint work_dims = 2;
    const size_t global_kernel_dims [work_dims] = {static_cast<size_t>(params.imheight), static_cast<size_t>(params.imwidth)};  
 
    auto start = std::chrono::high_resolution_clock::now();

    const cl_int2 constants = {{static_cast<cl_int>(params.MAX_ITER), static_cast<cl_int>(params.ORDER)}};
    const cl_int3 dimensions = {{static_cast<cl_int>(params.imheight), static_cast<cl_int>(params.imwidth), static_cast<cl_int>(params.imdepth)}};
    const cl_float3 flt_constants = {{params.MIN_LIMIT, params.MAX_LIMIT, params.BOUNDARY_VAL}};

    clSetKernelArg(ocl_kernel, 2, sizeof(cl_int3),  (void *)&dimensions);
    clSetKernelArg(ocl_kernel, 3, sizeof(cl_int2),  (void *)&constants);
    clSetKernelArg(ocl_kernel, 4, sizeof(cl_float3), (void *)&flt_constants);
    
    for (cl_int depth_idx = 0; depth_idx < params.imdepth; ++depth_idx)
    {
        clSetKernelArg(ocl_kernel, 0, sizeof(cl_mem),    (void *)&dev_image);
        clSetKernelArg(ocl_kernel, 1, sizeof(cl_int),    (void *)&depth_idx);

        auto ocl_error_num = clEnqueueNDRangeKernel(ocl_command_queue, ocl_kernel, work_dims, nullptr, global_kernel_dims, nullptr, 0, nullptr, nullptr);
        if(ocl_error_num != CL_SUCCESS)
            std::cout << "ERROR @ KERNEL LAUNCH -- " << ocl_error_num << " @depth " << depth_idx << std::endl;

        int h_image_stack_offset = params.imheight * params.imwidth * depth_idx;
        ocl_error_num = clEnqueueReadBuffer(ocl_command_queue, dev_image, CL_TRUE, 0, params.imheight * params.imwidth * sizeof(cldata_t), 
                                            &h_image_stack[h_image_stack_offset], 0, nullptr, nullptr);
        if(ocl_error_num != CL_SUCCESS)
            std::cout << "ERROR @ DATA RETRIEVE -- " << ocl_error_num << std::endl;

        if(verbose_run)
        {
          auto slice_sum = std::accumulate(&h_image_stack[h_image_stack_offset], &h_image_stack[h_image_stack_offset] + params.imheight * params.imwidth, 0);
          std::cout << "slice " << depth_idx << " sum: " << slice_sum << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    std::cout << "Fractal Generation Time: " << duration.count() << " ms" << std::endl;

    clReleaseKernel(ocl_kernel);
    clReleaseProgram(ocl_program);
    clReleaseMemObject(dev_image);
    clReleaseCommandQueue(ocl_command_queue);
    clReleaseContext(ocl_context);
}

#endif
