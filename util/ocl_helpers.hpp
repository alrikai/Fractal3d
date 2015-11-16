/* ocl_helpers.hpp -- part of the fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


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

//converts the pre-processor symbol for the preprocessor path (OCLKERNEL_FILEPATH) to a std::string
inline std::string get_kernelpath()
{
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

    //generate the buffer for the path string. Make sure it's null-terminated
    const size_t path_len = snprintf(NULL, 0, "%s", TOSTRING(OCLKERNEL_FILEPATH)) + 1;
    std::unique_ptr<char []> path_buffer = std::unique_ptr<char []> (new char[path_len]);
    std::fill(path_buffer.get(), path_buffer.get() + path_len, 0);
   
    //copy the path string to a C++ string
    snprintf(path_buffer.get(), path_len, "%s", TOSTRING(OCLKERNEL_FILEPATH));
    std::string data_filepath;
    data_filepath.assign(path_buffer.get());

#undef TOSTRING
#undef STRINGIFY

    return data_filepath;
}

} //namespace ocl_helpers

#endif
