#ifndef TD_CONTROLLER_HPP__
#define TD_CONTROLLER_HPP__

#include "InputListener.hpp"
#include "ControllerUtil.hpp"

#include <OIS/OIS.h>
#include <OGRE/Ogre.h>

#include <string>
#include <memory>

//listens for user input events
class Controller
{
public:
    typedef ControllerUtil::ControllerBufferType ControllerBufferType;

    Controller(Ogre::Root* root, Ogre::RenderWindow* ogre_window);
    ~Controller();

    void start();
    void window_resized(Ogre::RenderTarget* ogre_window);

    //for now: have clients register buffers with the controller, s.t. they'll 
    //recieve all the events of that particular type. We may want more fine-grained
    //event filtering in the future, but start with this for now.
    //NOTE: what will we do for temporal information? i.e. if we need to know the order
    //of the events, how will we correlate the keyboard events and the mouse events here?
    //Do we need to supply timestamps for everything?

    bool register_input_listener(std::string id, ControllerBufferType* buffer); 
private:
    void init();

    //not owned by the Controller
    Ogre::RenderWindow* window;

    std::unique_ptr<InputListener> listener;
    //OIS Input devices
    OIS::InputManager* ois_manager;
    OIS::Mouse* ois_mouse;
    OIS::Keyboard* ois_keyboard;

};

#endif
