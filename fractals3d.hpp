#ifndef OCL_FRACTALS_3D_HPP
#define OCL_FRACTALS_3D_HPP

#include <CL/cl.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <complex>
#include <memory>
#include <array>
#include <chrono>
#include <stdexcept>

#include "cpu_fractal.hpp"

namespace ocl_helpers
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

//note: some of these tricks require c++11 support
std::tuple<cl_uint, cl_platform_id, bool> get_platform_id(const std::string& target_platform)
{
    //get the number of available platforms
    cl_uint num_platforms;
    clGetPlatformIDs (0, nullptr, &num_platforms);
    
    //get the platform IDs
    std::vector<cl_platform_id> platform_IDs (num_platforms);
    clGetPlatformIDs(num_platforms, &platform_IDs[0], nullptr);

    //look for the target platform 
    cl_uint target_platform_ID = -1;
    bool found_target = false;
    for(cl_uint i = 0; i < num_platforms; ++i)
    {
        size_t platform_size;
        clGetPlatformInfo(platform_IDs[i], CL_PLATFORM_PROFILE, 0, nullptr, &platform_size);
        
        std::vector<char> curr_platform(platform_size);       
        clGetPlatformInfo (platform_IDs[i], CL_PLATFORM_NAME, platform_size, &curr_platform[0], nullptr);
        
        auto platform_profile = std::string(curr_platform.begin(), curr_platform.end());
        std::cout << "Platform @" << i << ": " << platform_profile << std::endl;
        if (platform_profile.find(target_platform) != std::string::npos) 
        {
            target_platform_ID = i;
            found_target = true;
            break;
        }
    }
    return std::make_tuple(target_platform_ID, platform_IDs[target_platform_ID], found_target);
}

bool load_kernel_file(const std::string& file_name, std::string& kernel_source)
{
    std::ifstream kernel_source_file(file_name);
    std::string str; 

    int i = 0;
    while (std::getline(kernel_source_file, str))
    {
        std::cout << "@ " << i++ << "  " << str << std::endl;
        kernel_source += str; 
    }

    return true;
}

struct fractal_params
{
  int imheight;
  int imwidth;
  int imdepth;

  static constexpr cl_int MAX_ITER = 80;
  static constexpr cl_int ORDER = 8;

  cl_float MIN_LIMIT;
  cl_float MAX_LIMIT;
  cl_float BOUNDARY_VAL;

  std::string fractal_name;
};

} //namespace ocl_helpers

template <typename data_t, template <class> class ptcloud_t, typename pt_t>
void make_pointcloud(const std::vector<data_t>& h_image_stack, const ocl_helpers::fractal_params& params, ptcloud_t<pt_t>& pt_cloud)
{
  auto start = std::chrono::high_resolution_clock::now();
  
  //for the cpu version
  bool run_comparison = !false;
  cpu_fractals::FractalLimits<data_t> limits(cpu_fractals::PixelPoint<size_t>(params.imheight, params.imwidth, params.imdepth));

  for (int k = 0; k < params.imdepth; ++k)
  {
      auto z_point = limits.offset_Z(k);

      cv::Mat_<int> slice_diff = cv::Mat_<int>::zeros(params.imheight, params.imwidth);
      cv::Mat_<int> ocl_slice = cv::Mat_<int>::zeros(params.imheight, params.imwidth);
      cv::Mat_<int> cpu_slice = cv::Mat_<int>::zeros(params.imheight, params.imwidth);
      
      int h_image_stack_offset = params.imheight * params.imwidth * k;
      const data_t* h_image_slice = &h_image_stack[h_image_stack_offset];
      for (int i = 0; i < params.imheight; ++i)
      {
          auto y_point = limits.offset_Y(i);
          for (int j = 0; j < params.imwidth; ++j)
          {
              auto fractal_itval = h_image_slice[i*params.imwidth+j];
              if(fractal_itval == params.MAX_ITER-1)
                  pt_cloud.push_back(pt_t(j,i,k));

              if(run_comparison)
              {
              auto x_point = limits.offset_X(j);
              bool is_valid;
              size_t iter_num;
              std::tie(is_valid, iter_num) = cpu_fractals::mandel_point<data_t,params.MAX_ITER>
                  (cpu_fractals::PixelPoint<data_t>(y_point,x_point,z_point), params.ORDER);  
               
              slice_diff(i,j) = iter_num - fractal_itval;  
              cpu_slice(i,j) = iter_num;
              }
              ocl_slice(i,j) = fractal_itval;
          }
      }

      if(run_comparison)
      {
      auto diff_extrema = std::minmax_element(slice_diff.begin(), slice_diff.end());
      auto min_diff = *diff_extrema.first;
      auto max_diff = *diff_extrema.second;
      if(max_diff > 1 || min_diff < -1)
          std::cout << "Slice " << k << " differed" << std::endl;

      std::string slice_diff_name = "slice_diff_" + std::to_string(k);
      cv::imwrite((slice_diff_name + ".png"), cv::abs(slice_diff));
      cv::FileStorage slice_storage((slice_diff_name + ".yml"), cv::FileStorage::WRITE);
      slice_storage << slice_diff_name << slice_diff;
      slice_storage << "ocl slice" << ocl_slice;
      slice_storage << "cpu slice" << cpu_slice;
      slice_storage.release();  
      std::string cpu_slice_name = "cpu_slice_" + std::to_string(k) + ".png";
      cv::imwrite(cpu_slice_name, cpu_slice);
      }        

      std::string ocl_slice_name = params.fractal_name + "_ocl_slice_" + std::to_string(k) + ".png";
      cv::imwrite(ocl_slice_name, ocl_slice);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<double, std::milli>(end - start);
  std::cout << "Fractal Comparison Time: " << duration.count() << " ms" << std::endl;   
}

template <typename data_t>
void run_ocl_fractal(std::vector<data_t>& h_image_stack, const ocl_helpers::fractal_params& params)
{
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

    const std::string file_name { "../ocl_fractal.cl" };
    std::string program_source;
    ocl_helpers::load_kernel_file(file_name, program_source);

    //create + compile the opencl program
    auto kernel_source_code = program_source.c_str();
    cl_program ocl_program = clCreateProgramWithSource(ocl_context, 1, (const char**) &kernel_source_code, 0, &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ PROGRAM CREATION -- " << ocl_error_num << std::endl;

    const std::string ocl_fractal_id = ocl_helpers::fractal_options::get_ocl_id(params.fractal_name);
    const std::string cl_opts {"-D FRACTAL_ID=" + ocl_fractal_id};
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
    }

    // Create kernel instance
    const std::string fractal_kernel_name = "fractal3d";
    cl_kernel ocl_kernel = clCreateKernel(ocl_program, fractal_kernel_name.c_str(), &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ KERNEL CREATION -- " << ocl_error_num << std::endl;
  
    cl_mem dev_image;
    dev_image = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY, params.imheight * params.imwidth * sizeof(cl_int), nullptr, 0);
   
    const cl_uint work_dims = 2;
    const size_t global_kernel_dims [work_dims] = {params.imheight, params.imwidth};  
 
    auto start = std::chrono::high_resolution_clock::now();

    const cl_int2 constants = {{params.MAX_ITER, params.ORDER}};
    const cl_int3 dimensions = {{params.imheight, params.imwidth, params.imdepth}};
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
        ocl_error_num = clEnqueueReadBuffer(ocl_command_queue, dev_image, CL_TRUE, 0, params.imheight * params.imwidth * sizeof(cl_int), 
                                            &h_image_stack[h_image_stack_offset], 0, 0, 0);
        if(ocl_error_num != CL_SUCCESS)
            std::cout << "ERROR @ DATA RETRIEVE -- " << ocl_error_num << std::endl;
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
