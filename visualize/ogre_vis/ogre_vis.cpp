#include <iostream>
#include <memory>
#include <chrono>
#include <string>


#include "ogre_util.hpp"

#include "Controller/Controller.hpp"
#include "Controller/ControllerUtil.hpp"
#include "Fractal3D.hpp"


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

Ogre::SceneNode* make_pointcloud_object(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, Ogre::SceneNode* map_node, 
        const std::vector<float>& target_location, const std::string cloud_name)
{
    Ogre::ManualObject* minimal_obj = scene_mgmt->createManualObject(cloud_name);

    const size_t MAX_ITER = 80;
    const int imheight = 64;
    const int imwidth = 64;
    const int imdepth = 64;
    const float pt_factor = 8.0f;
    std::vector<std::vector<float>> fractal_pts = make_fractal<unsigned char, double, MAX_ITER>(imheight, imwidth, imdepth, 8, pt_factor);

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
    
    float color_coeff = 1.0f/MAX_ITER;
    float alpha_coeff = 0.01f;    

    //get the coordinates to place the tower at
    const float height_offset = dim_avgs[0];
    const float width_offset = dim_avgs[1];
    const float depth_offset = dim_avgs[2];

    std::cout << "Naive Centroid: [" << height_offset << ", " << width_offset << ", " << depth_offset << "]" << std::endl;

    minimal_obj->begin("pointmaterial", Ogre::RenderOperation::OT_POINT_LIST);
    {
        for (auto pt : fractal_pts)
        {    
            minimal_obj->position(pt[0]- height_offset, pt[1] - width_offset, pt[2] - depth_offset);

            //we have to have the points that converged be solid, and the rest be semi-transparent
            if(pt[3] >= MAX_ITER-1)
                minimal_obj->colour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f));
            else
                minimal_obj->colour(Ogre::ColourValue(0.0f, color_coeff * pt[3], 0.0f, alpha_coeff * pt[3]));
        }
    }
    minimal_obj->end();
    auto child_node = map_node->createChildSceneNode();
    child_node->attachObject(minimal_obj);
    child_node->setPosition(target_location[0], target_location[1], target_location[2]);

    child_node->showBoundingBox(true);

    return child_node;
}

Ogre::SceneNode* make_map(Ogre::SceneManager* scene_mgmt, const std::string& mesh_name)
{
    //create a prefab plane to use as the map
    auto map_plane = scene_mgmt->createEntity(mesh_name, Ogre::SceneManager::PT_PLANE);
    map_plane->setMaterialName("Examples/GrassFloor");
    map_plane->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_1);
    auto map_node = scene_mgmt->getRootSceneNode()->createChildSceneNode(mesh_name);
    map_node->attachObject(map_plane);
    return map_node;
}


} //anon namespace

void start_display()
{   
    const std::string plugins_cfg_filename {"plugins.cfg"};
    std::unique_ptr<Ogre::Root> root (new Ogre::Root(plugins_cfg_filename));   

    //load resources
    const std::string resource_cfg_filename {"resources.cfg"};
    load_resources(resource_cfg_filename);
    
    //configure the system
    if(!root->restoreConfig())
        if(!root->showConfigDialog())
            return -1;

    Ogre::RenderWindow* render_window = root->initialise(true, "Minimal OGRE");
    Ogre::SceneManager* scene_mgmt = root->createSceneManager("OctreeSceneManager");
    //Ogre::SceneNode* root_node = scene_mgmt->getRootSceneNode();

    Ogre::Camera* camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(3);
    camera->setFarClipDistance(8000);
    //have a 4:3 aspect ratio
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));

    Ogre::Viewport* view_port = render_window->addViewport(camera);
    view_port->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    Ogre::Light* light = scene_mgmt->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);
    scene_mgmt->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));

    const std::string mesh_name {"GameMap"};
    Ogre::SceneNode* map_node = make_map(scene_mgmt, mesh_name);
    
    //create the skybox. Current skybox is pretty nonsensical, but I guess it's something?
    const std::string skybox_material {"Examples/SpaceSkyBox"};
    scene_mgmt->setSkyBox(true, skybox_material, 5000);
    view_port->setSkiesEnabled(true);

    ////////////////////////////////////////////////////////////////////////
    //feeble attempts at terrain 
    ////////////////////////////////////////////////////////////////////////
    //NOTE: should only do 1 of these per application
    //auto terrain_globalparams = new Ogre::TerrainGlobalOptions();
    //auto terrain_group = new Ogre::TerrainGroup(scene_mgmt, Ogre::Terrain::ALIGN_X_Y, 513, 12000.0f);
 
    HandleUserInput input_handler (root.get(), render_window, map_node);

    std::unique_ptr<MinimalWindowEventListener> window_event_listener(new MinimalWindowEventListener());
    Ogre::WindowEventUtilities::addWindowEventListener(render_window, window_event_listener.get());
    
    //@TODO: want to place these correctly on the game map, s.t. they'll always match correctly. 
    //eventually we'll have the GameMap class, which will subdivide the map region into tiles,
    //and when creating towers, the user clicks will be binned into these tiles, so that the tower
    //is snapped to have its origin in the center of the tile. 
    make_maplines(scene_mgmt, view_port, map_node);

    //try drawing something via point-cloud
    const std::string fractal_name {"minimal_fractal"};
    const std::vector<float> target_coord {0, 0, 0};
    auto fractal_node = make_pointcloud_object(scene_mgmt, view_port, map_node, target_coord, fractal_name);

    double time_elapsed = 0;   
    const double TOTAL_TIME = 60 * 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    do
    {
        root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();
      
        //how best to do this? check the input queues for messages? In this thread, or in another one?
        input_handler(scene_mgmt, view_port);            

        //////////////////////////////////////////////////////
        //for fun: try rotating a fractal
        fractal_node->yaw(Ogre::Radian(3.14159265/5000.0f));
        //////////////////////////////////////////////////////

        auto end_time = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
        time_elapsed = time_duration.count();
    } while(time_elapsed < TOTAL_TIME && !window_event_listener->close_display.load());


    Ogre::WindowEventUtilities::removeWindowEventListener(render_window, window_event_listener.get());        
    return 0;
}



