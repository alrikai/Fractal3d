#include "fractals3d.hpp"
//#include "fractals2d.hpp"
//#include "visualize/vtk_vis/mesh_vis.h"
#include "cpu_fractal.hpp"

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
void cpu_fractal2d (const std::string& fractal_id);

void cpu_fractal2d (const std::string& fractal_id)
{
  using data_t = int;
  fractal_params params;
  params.imheight = 128;
  params.imwidth = 128;
  params.imdepth = 128;
  params.MIN_LIMIT = -1.2f;
  params.MAX_LIMIT =  1.2f;

  params.BOUNDARY_VAL = 2.0f;
  params.fractal_name = fractal_id;

  //NOTE: need to dynamically allocate, as the memory requirements become prohibitive very fast (e.g. 512 x 512 x 512 of ints --> 4*2^27 bytes)
  std::vector<data_t> h_image_stack (params.imheight * params.imwidth * params.imdepth);
  std::fill(h_image_stack.begin(), h_image_stack.end(), 0);

  cpu_fractals::run_cpu_fractal<data_t>(h_image_stack, params);
}

void ocl_fractal3d (const std::string& fractal_id)
{
  using data_t = int;
  fractal_params params;
  params.imheight = 128;
  params.imwidth = 128;
  params.imdepth = 128;
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
{
  using data_t = int;
  fractal_params params;
  params.imheight = 128;
  params.imwidth = 128;
  params.imdepth = 128;
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

}
