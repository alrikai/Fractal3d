
#include "ogre_util.hpp"
#include <iostream>


namespace ogre_util
{

void load_resources(const std::string& resource_cfg_filename)
{
    Ogre::ConfigFile config;
    config.load(resource_cfg_filename);
    Ogre::ConfigFile::SectionIterator seci = config.getSectionIterator();
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}

//returns whether the click intersected the GameMap, and if so, what the distance was  
std::tuple<bool, float> check_point(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x, const float y)
{
    std::cout << "Viewport Actual Dim: [" << view_port->getActualLeft() << ", " << view_port->getActualTop() 
              << " -- " << view_port->getActualHeight() << ", " << view_port->getActualWidth() << "]" << std::endl;
    std::cout << "Viewport Relative  Dim: [" << view_port->getLeft() << ", " << view_port->getTop() 
              << " -- " << view_port->getHeight() << ", " << view_port->getWidth() << "]" << std::endl;
    
    auto cam = view_port->getCamera();
    Ogre::Ray ray = cam->getCameraToViewportRay(x,y);
    std::cout << "Ray [" << x << ", " << y << "] origin: " << ray.getOrigin() << " -- Point @ t=0: " << ray.getPoint(0.f) << " @direction " << ray.getDirection() << std::endl;
    std::cout << "Ray @ near plane: " << ray.getPoint(cam->getNearClipDistance()) << "Ray @ far plane: " << ray.getPoint(cam->getFarClipDistance()) << std::endl;

   
    std::tuple<bool, float> click_intersection {false, 0}; 
     
    auto r_query = scene_mgmt->createRayQuery(ray);
    r_query->setSortByDistance(true);
    auto& q_hits = r_query->execute();
    if(!q_hits.empty())
    {
        for (auto q_it = q_hits.begin(); q_it != q_hits.end(); ++q_it)
        {
            if(q_it->movable)
            {
                Ogre::MovableObject* obj = q_it->movable;
                std::cout << "moveable -- " << obj->getName() << ": " << obj->getMovableType() << " bounding: " << obj->getWorldBoundingBox() << std::endl;
                if(obj->getName() == "GameMap")
                {
                    std::get<0>(click_intersection) = true;
                    std::get<1>(click_intersection) = q_it->distance;
                    std::cout << "Ray Length: " << q_it->distance << std::endl;
                }
            }
            else if(q_it->worldFragment)
                std::cout << "world fragment" << std::endl;
            else
                std::cout << "???" << std::endl;
        }
    }
    else
        std::cout << "[" << x << ", " << y << "] Did not intersect with anything" << std::endl;

    return click_intersection;

/*
    Ogre::Vector2 cam_ray {x, y};
    Ogre::Plane full_plane = Ogre::Plane(Ogre::Vector3(cam->getDerivedOrientation().zAxis()), cam->getDerivedPosition());        
    Ogre::Ray plane_ray = cam->getCameraToViewportRay(cam_ray.x, cam_ray.y);
    std::cout << "Plane Ray [" << cam_ray.x << ", " << cam_ray.y << "] origin: " << plane_ray.getOrigin() << " -- Point @ t=0: " << plane_ray.getPoint(0.f) << std::endl;
    auto ray_result = plane_ray.intersects(full_plane);
    if(ray_result.first)
    {
        Ogre::Vector3 ray_pt = plane_ray.getPoint(ray_result.second);
        std::cout << "3D Point: [" << ray_pt[0] << ", " << ray_pt[1] << ", " << ray_pt[2] << "]" << std::endl;
    }
    else
        std::cout << cam_ray << " didnt intersect..." << std::endl;
*/        
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

}
