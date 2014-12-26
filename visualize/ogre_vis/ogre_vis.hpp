
#include <memory>
#include <atomic>
#include <chrono>

#include <OGRE/Ogre.h>
#include <boost/lockfree/spsc_queue.hpp>

#include "util/fractal_helpers.hpp"
#include "ogre_util.hpp"

class FractalOgre;

class OgreData : public Ogre::FrameListener, public Ogre::WindowEventListener
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


private:
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
	
	friend class ::FractalOgre;
};


class FractalOgre
{
public:
  typedef boost::lockfree::spsc_queue<fractal_genevent, boost::lockfree::capacity<128>> FractalBufferType;

  FractalOgre(const float rotate_factor, const float pan_factor)
    : ogre_data(plugins_cfg_filename, resource_cfg_filename, rotate_factor, pan_factor), current_fractal_node(nullptr)
  {
    fractal_evtbuffer = std::make_shared<FractalBufferType>();
    fractal_idx = 0;
  }

  inline std::shared_ptr<FractalBufferType> get_fractalgenevt_buffer()
  {
    return fractal_evtbuffer;
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

  void display_fractal (fractal_data&& fractal);

private:
  void display_loop()
  {
		HandleUserInput input_handler (ogre_data.root.get(), ogre_data.render_window, ogre_data.map_node);
    std::unique_ptr<MinimalWindowEventListener> window_event_listener(new MinimalWindowEventListener());
    Ogre::WindowEventUtilities::addWindowEventListener(ogre_data.render_window, window_event_listener.get());

    double time_elapsed = 0;   
    const double TOTAL_TIME = 60 * 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    do
    {
        ogre_data.root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();
      
        //how best to do this? check the input queues for messages? In this thread, or in another one?
        input_handler(ogre_data.scene_mgmt, ogre_data.view_port);            

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

  static const std::string resource_cfg_filename; 
  static const std::string plugins_cfg_filename; 
  static const std::string fractal_name;

  //keep track of the number of generated fractals to use in the point cloud IDs
  int fractal_idx;
  std::shared_ptr<FractalBufferType> fractal_evtbuffer;
  
  OgreData ogre_data;
  Ogre::SceneNode* current_fractal_node;
};



