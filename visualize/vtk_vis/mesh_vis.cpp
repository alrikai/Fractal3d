/* mesh_vis.cpp -- part of the fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "mesh_vis.h"

#include <pcl/io/pcd_io.h>
#include <pcl/io/vtk_io.h>

#include <pcl/features/normal_3d.h>
#include <pcl/surface/mls.h>
#include <pcl/surface/poisson.h>


#include <pcl/common/projection_matrix.h>
#include <pcl/surface/organized_fast_mesh.h>
#include <pcl/visualization/cloud_viewer.h>
#include <vtk-5.8/vtkSmartPointer.h>

#include <chrono>
#include <string>
#include <iostream>

void show_pointcloud(pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud, const std::string& ptcloud_id)
{
  pcl::visualization::CloudViewer viewer (ptcloud_id);
  viewer.showCloud (pt_cloud);
  while (!viewer.wasStopped ())
  {
  }
}

void show_model_mesh(pcl::PointCloud<pcl::PointXYZ>::Ptr pt_cloud, const std::string& mesh_fname)
{
    auto  start = std::chrono::high_resolution_clock::now();

    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
    ne.setInputCloud(pt_cloud);
    ne.setRadiusSearch (2);
  
    Eigen::Vector4f centroid;
    pcl::compute3DCentroid (*pt_cloud, centroid);
    ne.setViewPoint(centroid[0], centroid[1], centroid[2]);
  
    pcl::PointCloud<pcl::Normal>::Ptr cloud_normals (new pcl::PointCloud<pcl::Normal>);
    ne.compute (*cloud_normals);
  
    std::for_each(cloud_normals->begin(), cloud_normals->end(), []
              (pcl::Normal& pt)
              {
                pt.normal_x *= -1;
                pt.normal_y *= -1;
                pt.normal_z *= -1;
              });
  
    std::cout << "#cloud normals: " << cloud_normals->size() << std::endl;
      
    pcl::PointCloud<pcl::PointNormal>::Ptr smooth_pt_cloud_normals (new pcl::PointCloud<pcl::PointNormal>()); 
    pcl::concatenateFields (*pt_cloud, *cloud_normals, *smooth_pt_cloud_normals);
  
    pcl::Poisson<pcl::PointNormal> poisson;
    poisson.setDepth(12);
    poisson.setInputCloud(smooth_pt_cloud_normals);
    pcl::PolygonMesh mesh;
    poisson.reconstruct(mesh);
  
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    std::cout << "Mesh Generation Time: " << duration.count() << " ms" << std::endl;

    pcl::io::saveVTKFile (mesh_fname, mesh);
  
    pcl::visualization::PCLVisualizer viewer("Mesh View"); 
    viewer.addPolygonMesh(mesh);
    while(!viewer.wasStopped()) {
        viewer.spinOnce(); 
    }
}
