#include <CL/cl.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <complex>

namespace ocl_helpers
{
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
    cl_uint target_platform_ID;
    bool found_target = false;
    for(cl_uint i = 0; i < num_platforms; ++i)
    {
        size_t platform_size;
        clGetPlatformInfo(platform_IDs[i], CL_PLATFORM_PROFILE, 0, nullptr, &platform_size);
        
        std::vector<char> curr_platform(platform_size);       
        clGetPlatformInfo (platform_IDs[i], CL_PLATFORM_NAME, platform_size, &curr_platform[0], nullptr);
        
        auto platform_profile = std::string(curr_platform.begin(), curr_platform.end());
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

}

template <typename data_t>
void run_oclfractal(std::vector<data_t>& h_image_stack, const ocl_helpers::fractal_params& params)
{
    const std::string target_platform_id {"NVIDIA"};
    cl_uint ocl_platform;
    bool platform_present;
    cl_platform_id ocl_platform_id;

    std::tie(ocl_platform, ocl_platform_id, platform_present) = get_platform_id(target_platform_id);

    if(platform_present)
        std::cout << target_platform_id << " platform at index " << ocl_platform << std::endl;

    //5 things for every opencl host-side program:
    //1. cl_device_id
    //2. cl_command_queue
    //3. cl_context
    //4. cl_program
    //5. cl_kernel

    //get ONE GPU device on the target platform 
    const int num_gpu = 1;
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
    load_kernel_file(file_name, program_source);

    //create + compile the opencl program
    auto kernel_source_code = program_source.c_str();
    cl_program ocl_program = clCreateProgramWithSource(ocl_context, 1, (const char**) &kernel_source_code, 0, &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ PROGRAM CREATION -- " << ocl_error_num << std::endl;

    ocl_error_num = clBuildProgram(ocl_program, 0, 0, 0, 0, 0);
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
    cl_kernel ocl_kernel = clCreateKernel(ocl_program, "fractal2d", &ocl_error_num);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ KERNEL CREATION -- " << ocl_error_num << std::endl;

    const int imheight = 512;
    const int imwidth =  512;
    const float BOUNDARY_VAL = 2;
    const int MAX_ITER = 80;

    cl_uint h_image [imheight * imwidth];
    std::fill(std::begin(h_image), std::end(h_image), 0);
   
    cl_mem dev_image;
    dev_image = clCreateBuffer(ocl_context, CL_MEM_WRITE_ONLY,
            imheight * imwidth * sizeof(cl_uint), nullptr, 0);
    
    const float max_real = 1.0;
    cl_float2 min_limits = {{-2.0, -1.2}};
    cl_float2 max_limits = {{max_real, min_limits.s[1] + (max_real - min_limits.s[0] * imheight/imwidth)}};
    cl_float2 dev_factor = {{(max_limits.s[0] - min_limits.s[0]) / (imwidth-1), 
                             (max_limits.s[1] - min_limits.s[1]) / (imheight-1)}};

    //make the kernel arguments
    cl_float4 dev_limits = {{min_limits.s[0], min_limits.s[1], max_limits.s[0], max_limits.s[1]}};

    std::cout << "limits: " << dev_limits.s[0] << "  " << dev_limits.s[1] << "  " << dev_limits.s[2] << "  " << dev_limits.s[3] << std::endl;
    std::cout << "factor: " << dev_factor.s[0] << "  " << dev_factor.s[1] << std::endl;

    // Setup parameter values
    clSetKernelArg(ocl_kernel, 0, sizeof(cl_mem),    (void *)&dev_image);
    clSetKernelArg(ocl_kernel, 1, sizeof(cl_float4), (void *)&dev_limits);
    clSetKernelArg(ocl_kernel, 2, sizeof(cl_float2), (void *)&dev_factor);
    clSetKernelArg(ocl_kernel, 3, sizeof(cl_float),  (void *)&BOUNDARY_VAL);
    clSetKernelArg(ocl_kernel, 4, sizeof(cl_int),    (void *)&imwidth);
    clSetKernelArg(ocl_kernel, 5, sizeof(cl_int),    (void *)&MAX_ITER);

    const size_t cnBlockSize = imheight;
    const size_t cnBlocks = imwidth;
    const size_t cnDimension = cnBlocks * cnBlockSize;

    // Launch kernel
    ocl_error_num = clEnqueueNDRangeKernel(ocl_command_queue, ocl_kernel, 1, 0, &cnDimension, 0, 0, 0, 0);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ KERNEL LAUNCH -- " << ocl_error_num << std::endl;

    // Copy results from device back to host; block until complete
    ocl_error_num = clEnqueueReadBuffer(ocl_command_queue, dev_image, CL_TRUE, 0, cnDimension * sizeof(cl_uint), &h_image, 0, 0, 0);
    if(ocl_error_num != CL_SUCCESS)
        std::cout << "ERROR @ DATA RETRIEVE -- " << ocl_error_num << std::endl;

    cv::Mat output_image = cv::Mat::zeros (imheight, imwidth, CV_8UC3);
    for (int i = 0; i < imheight; ++i)
    {
        for (int j = 0; j < imwidth; ++j)
        {
            auto iter_num = h_image[i*imwidth+j];
            if(iter_num == MAX_ITER-1)
                output_image.at<cv::Vec3b>(i,j) = cv::Vec3b(0, 0, 0);   
            else
                output_image.at<cv::Vec3b>(i,j) = cv::Vec3b(5*iter_num, 5*iter_num, iter_num);   
        }
    }
    cv::imshow("Output Mandelbrot", output_image);
    cv::waitKey(0);

    clReleaseKernel(ocl_kernel);
    clReleaseProgram(ocl_program);
   
    clReleaseMemObject(dev_image);

    clReleaseCommandQueue(ocl_command_queue);
    clReleaseContext(ocl_context);

    return 0;
}
