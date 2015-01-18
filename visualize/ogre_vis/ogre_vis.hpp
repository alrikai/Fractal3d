
#include <memory>
#include <atomic>
#include <chrono>

#include <OGRE/Ogre.h>
#include <boost/lockfree/spsc_queue.hpp>

#include "util/fractal_helpers.hpp"
#include "ogre_util.hpp"

class FractalOgre
{
public:
	using data_t = float;
  typedef boost::lockfree::spsc_queue<fractal_genevent, boost::lockfree::capacity<128>> FractalBufferType;
  typedef boost::lockfree::spsc_queue<fractal_data<fractal_types::point_type, data_t>, boost::lockfree::capacity<128>> FractalDisplayBufferType;

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
  void display_fractal (fractal_data<point_t, data_t> fractal);

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
template <typename point_t>
void FractalOgre::display_fractal (fractal_data<point_t, FractalOgre::data_t> fractal)
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

