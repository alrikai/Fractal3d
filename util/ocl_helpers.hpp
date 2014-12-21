#ifndef FRACTAL_OCL_HELPERS_HPP
#define FRACTAL_OCL_HELPERS_HPP

#include <CL/cl.hpp>

#include <tuple>
#include <string>
#include <vector>
#include <fstream>

namespace ocl_helpers
{

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

  static constexpr int MAX_ITER = 80;
  static constexpr int ORDER = 8;

  cl_float MIN_LIMIT;
  cl_float MAX_LIMIT;
  cl_float BOUNDARY_VAL;

  std::string fractal_name;
};

} //namespace ocl_helpers

#endif
