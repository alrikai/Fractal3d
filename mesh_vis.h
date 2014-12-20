#ifndef MESH_VIS_H
#define MESH_VIS_H

#include <pcl/common/common.h>
#include <pcl/point_types.h>

void show_pointcloud(pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud, const std::string& ptcloud_id = "fractal pt cloud");
void show_model_mesh(pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud, const std::string&  mesh_fname = "mesh.vtk");

#endif
