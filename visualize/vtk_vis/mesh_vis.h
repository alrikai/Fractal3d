/* mesh_vis.h -- part of the fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef MESH_VIS_H
#define MESH_VIS_H
#include <pcl/common/common.h>
#include <pcl/point_types.h>

void show_pointcloud(pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud, const std::string& ptcloud_id = "fractal pt cloud");
void show_model_mesh(pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud, const std::string&  mesh_fname = "mesh.vtk");
#endif
