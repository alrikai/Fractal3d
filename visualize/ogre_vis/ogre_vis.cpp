#if 0

#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <stdexcept>

#include "ogre_util.hpp"
#include "ogre_vis.hpp"

#include "controller/Controller.hpp"
#include "controller/ControllerUtil.hpp"

template <typename pixel_t, typename data_t>
void FractalOgre<pixel_t, data_t>::display_loop()
{
  HandleUserInput input_handler (ogre_data.root.get(), ogre_data.render_window, ogre_data.map_node, ogre_data.cam_move, ogre_data.cam_rotate);
  std::unique_ptr<MinimalWindowEventListener> window_event_listener(new MinimalWindowEventListener());
  Ogre::WindowEventUtilities::addWindowEventListener(ogre_data.render_window, window_event_listener.get());

  double time_elapsed = 0;   
  const double TOTAL_TIME = 60 * 1000;
  auto start_time = std::chrono::high_resolution_clock::now();
  do
  {
      ogre_data.root->renderOneFrame();
      Ogre::WindowEventUtilities::messagePump();
    
      //handle any user inputs
      input_handler(ogre_data.scene_mgmt, ogre_data.view_port, fractal_evtbuffer);            

//---------------------------------------------------------------------------------------        

      //check for new rendering events
      fractal_data<fractal_types::point_type, pixel_t> fdata_evt;
      while(fractal_displayevtbuffer->pop(fdata_evt))
      {
        display_fractal(fdata_evt);
      }
     
//---------------------------------------------------------------------------------------        

      //////////////////////////////////////////////////////
      //for fun: try rotating a fractal
      if(current_fractal_node)
        current_fractal_node->yaw(Ogre::Radian(3.14159265/5000.0f));
      //////////////////////////////////////////////////////

      auto end_time = std::chrono::high_resolution_clock::now(); 
      std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
      time_elapsed = time_duration.count();
  } while(time_elapsed < TOTAL_TIME && !window_event_listener->close_display.load());

  Ogre::WindowEventUtilities::removeWindowEventListener(ogre_data.render_window, window_event_listener.get());        
}

//---------------------------------------------------------------------------------------        

template <typename pixel_t, typename data_t>
void FractalOgre<pixel_t, data_t>::OgreData::make_map(const std::string& mesh_name)
{
	//create a prefab plane to use as the map
	auto map_plane = scene_mgmt->createEntity(mesh_name, Ogre::SceneManager::PT_PLANE);
	map_plane->setMaterialName("Examples/GrassFloor");
	map_plane->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_1);
	map_node = scene_mgmt->getRootSceneNode()->createChildSceneNode(mesh_name);
	map_node->attachObject(map_plane);
}

template <typename pixel_t, typename data_t>
void FractalOgre<pixel_t, data_t>::OgreData::setup_lights()
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

template <typename pixel_t, typename data_t>
void FractalOgre<pixel_t, data_t>::OgreData::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(6000);
    //have a 4:3 aspect ratio, looking back along the Z-axis (should we do Y-axis instead?) 
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));
}

template <typename pixel_t, typename data_t>
void FractalOgre<pixel_t, data_t>::OgreData::ogre_setup()
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

template <typename pixel_t, typename data_t>
void FractalOgre<pixel_t, data_t>::OgreData::start_display(const std::string& map_materialname, const std::string& skybox_material)
{
	make_map(map_materialname);
	scene_mgmt->setSkyBox(true, skybox_material, 5000);
	view_port->setSkiesEnabled(true);	
}



#endif
