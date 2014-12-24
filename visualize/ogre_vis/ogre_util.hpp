#ifndef OGRE_VIS_OGRE_UTIL_HPP
#define OGRE_VIS_OGRE_UTIL_HPP

#include <Ogre.h>
#include <atomic>
#include <string>
#include <tuple>
#include <memory>


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
    HandleUserInput(Ogre::Root* root_node, Ogre::RenderWindow* render_window, Ogre::SceneNode* gamemap_node)
        : controller(root_node, render_window), listener_id ("minimal_listener"), map_node(gamemap_node)
    {
        input_events = std::unique_ptr<ControllerUtil::ControllerBufferType>(new ControllerUtil::ControllerBufferType());   
        controller.register_input_listener(listener_id, input_events.get());
        fractal_count = 0;
    }

    void operator() (Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port)
    {
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
                check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
            break;
            case ControllerUtil::INPUT_TYPE::RClick:
                std::cout << "Mouse Rclick @[" << ui_evt.x_pos/width << ", " << ui_evt.y_pos/height << "]" << std::endl;
                std::tie(valid_click, click_distance) = check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);

                if(valid_click)
                    place_fractal(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height, click_distance);
               break;
            case ControllerUtil::INPUT_TYPE::MDrag:
                std::cout << "Mouse Drag @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
            break;
            default: 
                std::cout << "Unknown Input Event" << std::endl;  
            }
        }
    }

    //x any are normalized screen coordinates
    void place_fractal(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x_coord, const float y_coord, const float click_distance)
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
        make_pointcloud_object(scene_mgmt, view_port, map_node, target_coord, fractal_name);
        ++fractal_count;
    }

    Controller controller;
    const std::string listener_id;
    std::unique_ptr<ControllerUtil::ControllerBufferType> input_events;
    int fractal_count;
  
    Ogre::SceneNode* map_node;
};



#endif
