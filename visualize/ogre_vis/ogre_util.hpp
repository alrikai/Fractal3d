#ifndef OGRE_VIS_OGRE_UTIL_HPP
#define OGRE_VIS_OGRE_UTIL_HPP

#include <atomic>
#include <string>
#include <tuple>
#include <memory>

#include <OGRE/Ogre.h>
#include "controller/Controller.hpp"
#include "util/fractal_helpers.hpp"

namespace ogre_util
{

	void load_resources(const std::string& resource_cfg_filename);
  std::tuple<bool, float> check_point(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x, const float y);

} //namespace ogre_util


class MinimalWindowEventListener : public Ogre::WindowEventListener
{
public:
    MinimalWindowEventListener()
        : close_display(false)
    {}

    void windowClosed(Ogre::RenderWindow* pRenderWindow)
    {
       Ogre::Root::getSingletonPtr()->queueEndRendering();
       close_display.store(true);
    }

    std::atomic<bool> close_display;
};

struct HandleUserInput
{
    HandleUserInput(Ogre::Root* root_node, Ogre::RenderWindow* render_window, Ogre::SceneNode* gamemap_node, const float cam_move, const float cam_rotate)
        : controller(root_node, render_window), listener_id ("fractal_listener"), map_node(gamemap_node), cam_move(cam_move), cam_rotate(cam_rotate)
    {
        input_events = std::unique_ptr<ControllerUtil::ControllerBufferType>(new ControllerUtil::ControllerBufferType());   
        controller.register_input_listener(listener_id, input_events.get());
        fractal_count = 0;
    }

    template <typename FractalBufferType>
    void operator() (Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, std::shared_ptr<FractalBufferType> fractal_evtbuffer)
    {
      ControllerUtil::InputEvent ui_evt;
      auto camera_direction = Ogre::Vector3::ZERO;

      //NOTE: need something a bit less shitty of a solution than this
      bool valid_click = false;
      float click_distance = 0;
      const float height = view_port->getActualHeight(); 
      const float width = view_port->getActualWidth();         

      //for mouse dragging, we can move as roll, pitch, yaw based on the mouse movement
      float cam_yaw = 0;
      float cam_pitch = 0;

      //take action based on the user input -- note: should we limit the user input rate?
      // or do this in another thread? 
      while(input_events->pop(ui_evt))
      {
          switch(ui_evt.event_type) 
          {
              case ControllerUtil::INPUT_TYPE::LArrow:
              {
                  camera_direction.x += -cam_move;
                  break;
              }
              case ControllerUtil::INPUT_TYPE::RArrow:
              {
                  camera_direction.x += cam_move;
                  break;
              }
              case ControllerUtil::INPUT_TYPE::UpArrow:
              { 
                  camera_direction.y += -cam_move;
                  break;
              }
              case ControllerUtil::INPUT_TYPE::DArrow:
              {
                  camera_direction.y += cam_move;
                  break;
              }
              case ControllerUtil::INPUT_TYPE::PDown:
              { 
                  camera_direction.z += -cam_move;
                  break;
              }
              case ControllerUtil::INPUT_TYPE::PUp:
              {
                  camera_direction.z += cam_move;
                  break;
              }
              case ControllerUtil::INPUT_TYPE::A:
                  std::cout << "Key A" << std::endl;    
              break;
              case ControllerUtil::INPUT_TYPE::S:
                  std::cout << "Key S" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::W:
                  std::cout << "Key W" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::D:
              {
  /*
                //have D mean deletion of the current object. This will likely change once we add a real GUI...
                  if(current_selection)
                  {
                      std::cout << "Deleting object " << current_selection->getName() << std::endl;
                      Ogre::SceneNode* t_scenenode = current_selection->getParentSceneNode();
                      OgreUtil::nuke_scenenode(t_scenenode);


                      //TODO: enqueue a tower delete event for the backend
                  }
  */                
                  break;
              }
              case ControllerUtil::INPUT_TYPE::ZERO:
                  std::cout << "Key 0" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::ONE:
                  std::cout << "Key 1" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::TWO:    
                  std::cout << "Key 2" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::THREE:    
                  std::cout << "Key 3" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::FOUR:    
                  std::cout << "Key 4" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::FIVE:    
                  std::cout << "Key 5" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::SIX:    
                  std::cout << "Key 6" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::SEVEN: 
                  std::cout << "Key 7" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::EIGHT:    
                  std::cout << "Key 8" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::NINE:
                  std::cout << "Key 9" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::Esc:
                  std::cout << "Esc Key" << std::endl;
              break;
              case ControllerUtil::INPUT_TYPE::LClick:
              {
                  std::cout << "Mouse Lclick @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                  //NOTE: current_selection is set to nullptr if nothing was selected. Might want to disallow selecting the game map and certain other objects
                  ogre_util::check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
                  break;
              }
              case ControllerUtil::INPUT_TYPE::RClick:
              {
                  std::cout << "Mouse Rclick @[" << ui_evt.x_pos/width << ", " << ui_evt.y_pos/height << "]" << std::endl;
                  std::tie(valid_click, click_distance) = ogre_util::check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);

                  if(valid_click)
                  {
                      auto fractal_prms = place_fractal(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height, click_distance);
                      fractal_evtbuffer->push(fractal_prms);
                  }
                 break;
              }
              case ControllerUtil::INPUT_TYPE::MDrag:
              {
                  //provides the difference in current mouse pos. from the previous mouse pos.
                  std::cout << "Mouse Drag @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                  cam_yaw = ui_evt.x_pos;
                  cam_pitch = ui_evt.y_pos;
                  break;
              }
              default:
                  std::cout << "Unknown Type" << std::endl;
          };

          ControllerUtil::print_input_type(ui_evt.event_type);
      }

      auto camera = view_port->getCamera();
      camera->move(camera_direction);
      camera->yaw(Ogre::Degree(cam_yaw)*-0.2f);
      camera->pitch(Ogre::Degree(cam_pitch)*-0.2f);
    }

    //x any are normalized screen coordinates
    fractal_genevent place_fractal(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x_coord, const float y_coord, const float click_distance)
    {
        auto cam = view_port->getCamera();
        std::cout << "Camera Position: " << cam->getDerivedPosition() << "Camera Direction: " << cam->getDerivedDirection() 
                  << "Pixel Ratio: " << cam->getPixelDisplayRatio() << std::endl;
        //get the normalized world coordinates of the click at Z = 0
        Ogre::Ray ray = cam->getCameraToViewportRay(x_coord, y_coord);
    
        //TODO: execute a scene query with the ray and find the intersection point with the world geometry
        //then find the intersection point of the ray and the world; that'll be the point at which the 
        //tower is constructed at. --> alternately, the RaySceneQueryResultEntry object has a field "distance"
        //which is the distance along the ray to the point of intersection, which we would use with the ray.getPoint call.

        //NOTE: this might be where the problem is; since we're always taking the Z-distance of the camera, we might be 
        //not going far enough from points not along the camera ray

//cam->getDerivedPosition().z
        //ray.setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);
        auto world_click = ray.getPoint(click_distance);
        
        const std::string fractal_name {"rclickminimal_fractal_" + std::to_string(fractal_count)};
        //what to do about the Z-coord? We would want to have it be the map-plane's z-val
        const std::vector<float> target_coord {world_click[0], world_click[1], 0};
        
        //TODO: we'll want some sort of GUI for selecting parameters, etc.
        fractal_params params;
        params.imheight  = 128;
        params.imwidth   = 129;
        params.imdepth   = 128;
        params.MIN_LIMIT = -1.2f;
        params.MAX_LIMIT =  1.2f;
        params.BOUNDARY_VAL = 2.0f;
        params.fractal_name = "mandelbrot";

        ++fractal_count;
        return params;
    }

    Controller controller;
    const std::string listener_id;
    std::unique_ptr<ControllerUtil::ControllerBufferType> input_events;
    int fractal_count;
    const float cam_move;
    const float cam_rotate;
    Ogre::SceneNode* map_node;
};

/*
        const float height = view_port->getActualHeight(); 
        const float width = view_port->getActualWidth(); 

        //NOTE: need something a bit less shitty of a solution than this
        bool valid_click = false;
        float click_distance = 0;       

        ControllerUtil::InputEvent ui_evt;
        while(input_events->pop(ui_evt))    
        {
            switch(ui_evt.event_type)
            {
            case ControllerUtil::INPUT_TYPE::LClick:
                std::cout << "Mouse Lclick @[" << ui_evt.x_pos/width<< ", " << ui_evt.y_pos/height << "]" << std::endl;
								ogre_util::check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
            break;
            case ControllerUtil::INPUT_TYPE::RClick:
                std::cout << "Mouse Rclick @[" << ui_evt.x_pos/width << ", " << ui_evt.y_pos/height << "]" << std::endl;
                std::tie(valid_click, click_distance) = ogre_util::check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);

                if(valid_click)
                {
                    auto fractal_prms = place_fractal(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height, click_distance);
                    fractal_evtbuffer->push(fractal_prms);
                }
               break;
            case ControllerUtil::INPUT_TYPE::MDrag:
                std::cout << "Mouse Drag @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
            break;
            default: 
                std::cout << "Unknown Input Event" << std::endl;  
            }
        }
*/

#endif

