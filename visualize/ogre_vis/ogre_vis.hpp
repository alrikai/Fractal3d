
#include <memory>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <stdexcept>

#include <OGRE/Ogre.h>
#include <boost/lockfree/spsc_queue.hpp>

#include "util/fractal_helpers.hpp"
#include "ogre_util.hpp"


#include "controller/Controller.hpp"
#include "controller/ControllerUtil.hpp"
template <typename pixel_t>
class FractalOgre
{
public:
  typedef boost::lockfree::spsc_queue<fractal_genevent, boost::lockfree::capacity<128>> FractalBufferType;
  typedef boost::lockfree::spsc_queue<fractal_data<fractal_types::point_type, pixel_t>, boost::lockfree::capacity<128>> FractalDisplayBufferType;

  FractalOgre(const float rotate_factor, const float pan_factor)
    : ogre_data(plugins_cfg_filename, resource_cfg_filename, rotate_factor, pan_factor), current_fractal_node(nullptr)
  {
    fractal_evtbuffer = std::make_shared<FractalBufferType>();
    fractal_displayevtbuffer = std::make_shared<FractalDisplayBufferType>();

    fractal_idx = 0;
  }

  inline std::shared_ptr<FractalBufferType> get_fractalgenevt_buffer()
  {
    return fractal_evtbuffer;
  }

  inline std::shared_ptr<FractalDisplayBufferType> get_fractaldispevt_buffer()
  {
    return fractal_displayevtbuffer;
  }

  void start_display()
  {
    const std::string map_materialname {"GameMap"}; 
    //create the skybox. Current skybox is pretty nonsensical, but I guess it's something?
    const std::string skybox_material {"Examples/SpaceSkyBox"};
		ogre_data.start_display(map_materialname, skybox_material);
	
    //enters the display loop -- is blocking call, uses calling thread
    display_loop();
  }

  template <typename point_t = fractal_types::point_type>
  void display_fractal (fractal_data<point_t, pixel_t> fractal);

private:
  struct OgreData : public Ogre::FrameListener, public Ogre::WindowEventListener
  {
  public:
    OgreData(const std::string& plugins_cfg_filename, const std::string& resource_cfg_filename, float rotate_factor = 0.10f, float pan_factor = 10.0f)
        : root (new Ogre::Root(plugins_cfg_filename)), resource_cfg_filename(resource_cfg_filename),cam_rotate(rotate_factor), cam_move(pan_factor), close_display(false)
    {
        ogre_setup();
        setup_camera(); 
        setup_lights();

        //not really sure if these should get their own method, or be rolled into an existing one. So for now, just have them here...
        view_port = render_window->addViewport(camera);
        view_port->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
       
        //background.reset(new GameBackground(scene_mgmt, view_port));
        //input_events = std::unique_ptr<ControllerUtil::ControllerBufferType>(new ControllerUtil::ControllerBufferType());
        
        Ogre::Root::getSingletonPtr()->addFrameListener(this);
    }

    void start_display(const std::string& mapmesh_name, const std::string& skybox_material);

    void ogre_setup();
    void setup_camera(); 
    void setup_lights();
    void make_map(const std::string& mesh_name);

    std::unique_ptr<Ogre::Root> root;
    const std::string resource_cfg_filename;

    Ogre::RenderWindow* render_window;
    Ogre::SceneManager* scene_mgmt;
    Ogre::SceneNode* root_node;
    Ogre::SceneNode* map_node;

    //the coefficients for camera movement (there will be more as we add more functionality)
    const float cam_rotate;
    const float cam_move;

    Ogre::Camera* camera;
    Ogre::Viewport* view_port;
    Ogre::Light* main_light;
    Ogre::Light* spot_light;
    
    std::atomic<bool> close_display;
  };
  
  void display_loop();
  
  static const std::string resource_cfg_filename; 
  static const std::string plugins_cfg_filename; 
  static const std::string fractal_name;

  //keep track of the number of generated fractals to use in the point cloud IDs
  int fractal_idx;
  std::shared_ptr<FractalBufferType> fractal_evtbuffer;
  std::shared_ptr<FractalDisplayBufferType> fractal_displayevtbuffer;

  OgreData ogre_data;
  Ogre::SceneNode* current_fractal_node;
};


//draw the input fractal to the display
template <typename pixel_t>
template <typename point_t>
void FractalOgre<pixel_t>::display_fractal (fractal_data<point_t, pixel_t> fractal)
{
  //put the fractal in the middle of the scene
  const std::vector<float> target_coord = fractal.target_coord;
  const float pt_factor = 2.0f;

  auto fractal_pts = fractal.point_cloud.cloud;
  std::vector<float> dim_avgs (3, 0);
  std::for_each(fractal_pts.begin(), fractal_pts.end(), [&dim_avgs]
        (const point_t& pt)
        {
            dim_avgs[0] += pt.x;
            dim_avgs[1] += pt.y;
            dim_avgs[2] += pt.z;
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

  const std::string cloud_name = fractal_name + "_" + std::to_string(fractal_idx);
 
  //auto fractal_obj = ogre_data.scene_mgmt->createEntity(cloud_name, Ogre::SceneManager::PT_SPHERE);
  //fractal_obj->setMaterialName("pointmaterial");
 
  Ogre::ManualObject* fractal_obj = ogre_data.scene_mgmt->createManualObject(cloud_name);
  fractal_obj->begin("pointmaterial", Ogre::RenderOperation::OT_POINT_LIST);
  {
      for (auto pt : fractal_pts)
      {   
			  if(pt.x < 0 || pt.y < 0 || pt.z < 0)
					std::cout << "NOTE: pt is bad -- [" << pt.x << ", " << pt.y << ", " << pt.z << "]" << std::endl; 

          fractal_obj->position(pt.x - height_offset, pt.y - width_offset, pt.z - depth_offset);

          //we have to have the points that converged be solid, and the rest be semi-transparent
          if(pt.value >= fractal.params.MAX_ITER-1)
              fractal_obj->colour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f));
          else
              fractal_obj->colour(Ogre::ColourValue(0.0f, color_coeff * pt.value, 0.0f, alpha_coeff * pt.value));
      }
  }
  fractal_obj->end();
  

  //TODO: need to remove the old fractal's scenenode to cleanup -- look into how best to do this

  auto new_fractal_node = ogre_data.map_node->createChildSceneNode();
  new_fractal_node->attachObject(fractal_obj);
  new_fractal_node->setPosition(target_coord[0], target_coord[1], target_coord[2]);

  new_fractal_node->scale(1.0f/pt_factor, 1.0f/pt_factor, 1.0f/pt_factor);
  

  new_fractal_node->showBoundingBox(true);
  fractal_idx++;

  //out with the old, in with the new...
  current_fractal_node = new_fractal_node;
}


template <typename pixel_t>
const std::string FractalOgre<pixel_t>::resource_cfg_filename {"resources.cfg"}; 

template <typename pixel_t>
const std::string FractalOgre<pixel_t>::plugins_cfg_filename {"plugins.cfg"}; 

template <typename pixel_t>
const std::string FractalOgre<pixel_t>::fractal_name {"minimal_fractal"};


template <typename pixel_t>
void FractalOgre<pixel_t>::display_loop()
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

template <typename pixel_t>
void FractalOgre<pixel_t>::OgreData::make_map(const std::string& mesh_name)
{
	//create a prefab plane to use as the map
	auto map_plane = scene_mgmt->createEntity(mesh_name, Ogre::SceneManager::PT_PLANE);
	map_plane->setMaterialName("Examples/GrassFloor");
	map_plane->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_1);
	map_node = scene_mgmt->getRootSceneNode()->createChildSceneNode(mesh_name);
	map_node->attachObject(map_plane);
}

template <typename pixel_t>
void FractalOgre<pixel_t>::OgreData::setup_lights()
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

template <typename pixel_t>
void FractalOgre<pixel_t>::OgreData::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(6000);
    //have a 4:3 aspect ratio, looking back along the Z-axis (should we do Y-axis instead?) 
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));
}

template <typename pixel_t>
void FractalOgre<pixel_t>::OgreData::ogre_setup()
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

template <typename pixel_t>
void FractalOgre<pixel_t>::OgreData::start_display(const std::string& map_materialname, const std::string& skybox_material)
{
	make_map(map_materialname);
	scene_mgmt->setSkyBox(true, skybox_material, 5000);
	view_port->setSkiesEnabled(true);	
}
