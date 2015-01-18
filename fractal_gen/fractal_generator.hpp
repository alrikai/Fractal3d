#include <iostream>
#include <algorithm>

#include <CL/cl.hpp>
#include "util/fractal_helpers.hpp"
#include "fractals3d.hpp"

class oclFractals
{
public:
  using point_t = fractal_types::point_type;
  using data_t = float;

  oclFractals()
  {}

  fractal_data<point_t, data_t> make_fractal(fractal_params&& fractalgen_params)
  {
    //NOTE: need to dynamically allocate, as the memory requirements become prohibitive very fast (e.g. 512 x 512 x 512 of ints --> 4*2^27 bytes)
    std::vector<data_t> h_image_stack (fractalgen_params.imheight * fractalgen_params.imwidth * fractalgen_params.imdepth);
    std::fill(h_image_stack.begin(), h_image_stack.end(), 0);

    run_ocl_fractal<data_t>(h_image_stack, fractalgen_params);
    std::cout << "Making Point Cloud... " << std::endl;

  //-------------------------------------------------------

    fractal_data<point_t, data_t> fdata;
//-----------------------------------------------------------------------------------------------------------------------    
    make_pointcloud<fractal_types::pointcloud, point_t, data_t> (h_image_stack, fractalgen_params, fdata.point_cloud);
//-----------------------------------------------------------------------------------------------------------------------    
/*
    //mock-up the generation to make a pseudo-point cloud
    for (int x = 10; x < 110; ++x)
    {
      for (int y = 10; y < 110; ++y)
      {
        for (int z = 10; z < 110; ++z)
        {
          fdata.point_cloud.emplace_back(x, y, z, 79);
        }
      }
    }
*/    
//-----------------------------------------------------------------------------------------------------------------------    
    fdata.params = fractalgen_params;

    return fdata;
  }
};

