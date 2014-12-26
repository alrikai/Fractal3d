#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <stdexcept>

#include "ogre_util.hpp"
#include "ogre_vis.hpp"

#include "controller/Controller.hpp"
#include "controller/ControllerUtil.hpp"

namespace {

void make_maplines(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, Ogre::SceneNode* map_node)
{
    const int height = view_port->getActualHeight(); 
    const int width = view_port->getActualWidth(); 
    std::cout << "Viewport Dimensions: " << height << " x " << width << std::endl;
    std::cout << "Viewport Real Dims: " << view_port->getHeight() << " x " << view_port->getWidth() << std::endl;

    const int mapheight = 0.75 * height;
    const int mapwidth = 0.75 * width;
    const float dim_ratio = mapwidth / static_cast<float>(mapheight);
    std::cout << "Map Dimensions: " << mapheight << " x " << mapwidth << std::endl;


//////////////////////////////////////////////////////////////////////////////////////
//need to look at the camera and the viewport to see where things should be placed
    
    auto cam = view_port->getCamera();

    //corners are ordered as follows: top-right near, top-left near, bottom-left near, bottom-right near, top-right far, top-left far, bottom-left far, bottom-right far
    auto cam_world_coords = cam->getWorldSpaceCorners();
    std::cout << "cam. world coords: top-right near, top-left near, bottom-left near, bottom-right near, top-right far, top-left far, bottom-left far, bottom-right far" << std::endl;
    for (int i = 0; i < 8; ++i)
        std::cout << *(cam_world_coords+i) << std::endl;

    auto target = view_port->getTarget();
    std::cout << "target dimensions: " << target->getHeight() << " x " << target->getWidth() << std::endl;

//////////////////////////////////////////////////////////////////////////////////////

    const float tileheight = 50.0f;
    const float tilewidth = 50.0f;

    const int num_rowtiles = mapheight / tileheight;
    const int num_coltiles = mapwidth / tilewidth;
    const std::string mapgrid_material {"BaseWhiteNoLighting"};

    //TODO: figure out how we arrive at these numbers (sans trial and error...)
    //since they still follow the screen dimensions properly, how do we arrive 
    //at these #s? the width of the map is apparently 67 of these units (i.e. 
    //[-33, 33]). The pixel-width of the map is 600. 
    //Is it something with the viewport or camera position?
    const float col_offset = 33;
    const float col_step = std::abs(2*col_offset) / num_coltiles;
    const float row_offset = -col_offset / dim_ratio;
    const float row_step = std::abs(2*row_offset) / num_rowtiles;

    //first, draw a single line
    std::vector<Ogre::ManualObject*> map_grid_rowlines (num_rowtiles+1);
    std::vector<Ogre::SceneNode*> row_line_nodes (num_rowtiles+1);
    for (int row = 0; row < num_rowtiles+1; ++row)
    {
        const std::string row_line_name = "mapline_row_" + std::to_string(row);
        map_grid_rowlines.at(row) = scene_mgmt->createManualObject(row_line_name);
        map_grid_rowlines.at(row)->begin(mapgrid_material, Ogre::RenderOperation::OT_LINE_LIST); 
        {
            //make the start and end points for the row lines
            map_grid_rowlines.at(row)->position(-col_offset, row_offset + row * row_step, 0);
            map_grid_rowlines.at(row)->position( col_offset, row_offset + row * row_step, 0);   
        }
        map_grid_rowlines.at(row)->end();
        map_grid_rowlines.at(row)->setRenderQueueGroup(Ogre::RENDER_QUEUE_1);     
   
        row_line_nodes.at(row) = map_node->createChildSceneNode();
        row_line_nodes.at(row)->attachObject(map_grid_rowlines.at(row));
    }

    std::vector<Ogre::ManualObject*> map_grid_collines (num_coltiles+1);
    std::vector<Ogre::SceneNode*> col_line_nodes (num_coltiles+1);
    
    for (int col = 0; col < num_coltiles+1; ++col)
    {
        const std::string col_line_name = "mapline_col_" + std::to_string(col);
        map_grid_collines.at(col) = scene_mgmt->createManualObject(col_line_name);
        map_grid_collines.at(col)->begin(mapgrid_material, Ogre::RenderOperation::OT_LINE_LIST); 
        {
            //make the start and end points for the col lines
            map_grid_collines.at(col)->position(-col_offset + col * col_step, -row_offset, 0);
            map_grid_collines.at(col)->position(-col_offset + col * col_step,  row_offset, 0);   
        }
        map_grid_collines.at(col)->end();
        map_grid_collines.at(col)->setRenderQueueGroup(Ogre::RENDER_QUEUE_1);     

        col_line_nodes.at(col) = map_node->createChildSceneNode();
        col_line_nodes.at(col)->attachObject(map_grid_collines.at(col));
    }

    /* Have a list of unresolved questions to address:
     * 
     * 1. do we use normalized coordinates? Or can we do it based on pixel values?
     * 2. how do we place an object in the scene? Do we use global positioning until we attach it to a scene node?
     */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Ogre::Vector3 position {33, 33/dim_ratio, 0};
    auto proj_pos = cam->getProjectionMatrix() * (cam->getViewMatrix() * position);
    Ogre::Vector2 screen_pos = Ogre::Vector2::ZERO;
    screen_pos.x = (proj_pos.x / 2.f) + 0.5f;
    screen_pos.y = (proj_pos.y / 2.f) + 0.5f;
    std::cout << "WORLD: [" << position[0] << ", " << position[1] << ", " << position[2] << "] --> SCREEN: [" << screen_pos.x << ", " << screen_pos.y << "]" << std::endl;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}


} //anon namespace

//draw the input fractal to the display
void FractalOgre::display_fractal (fractal_data&& fractal)
{
  //put the fractal in the middle of the scene
  const std::vector<float> target_coord {0.0f, 0.0f, 0.0f};
  const std::string cloud_name = fractal_name + "_" + std::to_string(fractal_idx);
  Ogre::ManualObject* fractal_obj = ogre_data.scene_mgmt->createManualObject(cloud_name);

  const float pt_factor = 8.0f;

  auto fractal_pts = fractal.point_cloud;
  std::vector<float> dim_avgs (3, 0);
  std::for_each(fractal_pts.begin(), fractal_pts.end(), [&dim_avgs]
        (const std::vector<float>& pt)
        {
            dim_avgs[0] += pt[0];
            dim_avgs[1] += pt[1];
            dim_avgs[2] += pt[2];
        });

  //get the average coordinate
  dim_avgs[0] /= fractal_pts.size();
  dim_avgs[1] /= fractal_pts.size();
  dim_avgs[2] /= fractal_pts.size();

  std::cout << "Fractal Centroid: [" << dim_avgs[0] << ", " << dim_avgs[1] << ", " << dim_avgs[2] << "]" << std::endl; 
    
  float color_coeff = 1.0f / fractal.params.MAX_ITER;
  float alpha_coeff = 0.01f;    

  //get the coordinates to place the tower at
  const float height_offset = dim_avgs[0];
  const float width_offset = dim_avgs[1];
  const float depth_offset = dim_avgs[2];

  std::cout << "Naive Centroid: [" << height_offset << ", " << width_offset << ", " << depth_offset << "]" << std::endl;

  fractal_obj->begin("pointmaterial", Ogre::RenderOperation::OT_POINT_LIST);
  {
      for (auto pt : fractal_pts)
      {    
          fractal_obj->position(pt[0]- height_offset, pt[1] - width_offset, pt[2] - depth_offset);

          //we have to have the points that converged be solid, and the rest be semi-transparent
          if(pt[3] >= fractal.params.MAX_ITER-1)
              fractal_obj->colour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f));
          else
              fractal_obj->colour(Ogre::ColourValue(0.0f, color_coeff * pt[3], 0.0f, alpha_coeff * pt[3]));
      }
  }
  fractal_obj->end();

  //TODO: need to remove the old fractal's scenenode to cleanup -- look into how best to do this

  auto new_fractal_node = ogre_data.map_node->createChildSceneNode();
  new_fractal_node->attachObject(fractal_obj);
  new_fractal_node->setPosition(target_coord[0], target_coord[1], target_coord[2]);

  new_fractal_node->showBoundingBox(true);
  fractal_idx++;

  //out with the old, in with the new...
  current_fractal_node = new_fractal_node;
}

void OgreData::make_map(const std::string& mesh_name)
{
	//create a prefab plane to use as the map
	auto map_plane = scene_mgmt->createEntity(mesh_name, Ogre::SceneManager::PT_PLANE);
	map_plane->setMaterialName("Examples/GrassFloor");
	map_plane->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_1);
	map_node = scene_mgmt->getRootSceneNode()->createChildSceneNode(mesh_name);
	map_node->attachObject(map_plane);
}

void OgreData::setup_lights()
{
  main_light = scene_mgmt->createLight("MainLight");
  main_light->setType(Ogre::Light::LightTypes::LT_DIRECTIONAL);
  main_light->setDiffuseColour(Ogre::ColourValue(.25, .25, 0));
  main_light->setSpecularColour(Ogre::ColourValue(.25, .25, 0));
  main_light->setDirection(Ogre::Vector3(0,-1,1));

  spot_light = scene_mgmt->createLight("OtherLight");
  spot_light->setPosition(20.0f, 80.0f, 50.0f); 

  Ogre::Light* spotLight = scene_mgmt->createLight("spotLight");
  spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
  spotLight->setDiffuseColour(0, 0, 1.0);
  spotLight->setSpecularColour(0, 0, 1.0);
  spotLight->setDirection(-1, -1, 0);
  spotLight->setPosition(Ogre::Vector3(0, 0, 300));
  spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
}

void OgreData::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(6000);
    //have a 4:3 aspect ratio, looking back along the Z-axis (should we do Y-axis instead?) 
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));
}

void OgreData::ogre_setup()
{
    //load resources
	ogre_util::load_resources(resource_cfg_filename);
    
    //configure the system
    if(!root->restoreConfig())
        if(!root->showConfigDialog())
            throw std::runtime_error("Could not configure Ogre system");

    render_window = root->initialise(true, "Fractal OGRE");
    scene_mgmt = root->createSceneManager("OctreeSceneManager");
    root_node = scene_mgmt->getRootSceneNode();
}

void OgreData::start_display(const std::string& map_materialname, const std::string& skybox_material)
{
	make_map(map_materialname);
	scene_mgmt->setSkyBox(true, skybox_material, 5000);
	view_port->setSkiesEnabled(true);	
}

const std::string FractalOgre::resource_cfg_filename {"resources.cfg"}; 
const std::string FractalOgre::plugins_cfg_filename {"plugins.cfg"}; 
const std::string FractalOgre::fractal_name {"minimal_fractal"};


