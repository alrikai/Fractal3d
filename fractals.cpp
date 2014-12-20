#include "fractals3d.hpp"
//#include "fractals2d.hpp"
#include "mesh_vis.h"

#include <vector>
#include <string>
#include <iostream>

struct point_type
{
  point_type()
    : x(0), y(0), z(0)
  {}

  point_type(int x_coord, int y_coord, int z_coord)
    : x(x_coord), y(y_coord), z(z_coord)
  {}

  int x, y, z;
};

template <typename point_t>
struct pointcloud
{
  void push_back(point_t pt)
  {
    cloud.push_back(pt);
  }

  std::vector<point_t> cloud; 
};

void ocl_fractal3d (const std::string& fractal_id);
void ocl_fractal2d (const std::string& fractal_id);

int main(int argc, char* argv[])
{
  if(argc <= 1)
  {
    std::cout << "Invalid Arguments -- Usage: <2D | 3D> <fractal_name>" << std::endl;
    return 1;
  }

  std::vector<std::string> user_args (argv+1, argv + argc);

  int arg_cnt = 0;
  std::for_each(user_args.begin(), user_args.end(), [&arg_cnt](const std::string& arg)
      {
        std::cout << "@ arg " << arg_cnt << ": " << arg << std::endl;
        arg_cnt++;
      });

  auto fracids = ocl_helpers::fractal_options::get_ids();
  auto fractal_id = std::find(fracids.begin(), fracids.end(), user_args.at(1));
  if(fractal_id != fracids.end())
  {
    std::cout << "Using fractal " << *fractal_id << std::endl;
  }
  else
  {
    std::cout << "NOTE: Invalid fractal selected. Possible options: " << std::endl;
    std::for_each(user_args.begin(), user_args.end(), [](const std::string& arg)
    {
      std::cout << arg << std::endl;
    });
    return 1;
  }

  if(user_args.at(0) == "2d" || user_args.at(0) == "2D")
    ocl_fractal2d (*fractal_id);
  else if(user_args.at(0) == "3d" || user_args.at(0) == "3D")
    ocl_fractal3d (*fractal_id);
  else
    std::cout << "NOTE: fractal dimension invalid (we got both flavors here -- 2D and 3D)" << std::endl;
    return 1;
  
  return 0;
}

void ocl_fractal3d (const std::string& fractal_id)
{
  using data_t = int;
  ocl_helpers::fractal_params params;
  params.imheight = 4*128;
  params.imwidth = 4*128;
  params.imdepth = 4*128;
  params.MIN_LIMIT = -1.2f;
  params.MAX_LIMIT =  1.2f;

  params.BOUNDARY_VAL = 2.0f;
  params.fractal_name = fractal_id;

  //NOTE: need to dynamically allocate, as the memory requirements become prohibitive very fast (e.g. 512 x 512 x 512 of ints --> 4*2^27 bytes)
  std::vector<data_t> h_image_stack (params.imheight * params.imwidth * params.imdepth);
  std::fill(h_image_stack.begin(), h_image_stack.end(), 0);

  run_ocl_fractal<data_t>(h_image_stack, params);
  std::cout << "Making Point Cloud... " << std::endl;

//-------------------------------------------------------
  
//  using ptcloud_t = pcl::PointCloud<pcl::PointXYZ>::Ptr;
//  ptcloud_t pt_cloud (new pcl::PointCloud<pcl::PointXYZ>());

  using pt_t = point_type;
  pointcloud<pt_t> pt_cloud;
  make_pointcloud<data_t, pointcloud, pt_t> (h_image_stack, params, pt_cloud);

  //show_pointcloud(pt_cloud);
  //std::string mesh_fname = "mesh" + params.fractal_name + ".vtk";
  //show_model_mesh(pt_cloud, mesh_fname); 
}


void ocl_fractal2d (const std::string& fractal_id)
{}
