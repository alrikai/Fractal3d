#include "Controller.hpp"

Controller::Controller(Ogre::Root* root, Ogre::RenderWindow* ogre_window)
    : window(ogre_window), ois_manager(nullptr), ois_mouse(nullptr), ois_keyboard(nullptr)
{
    init();
 
    listener = std::unique_ptr<InputListener>(new InputListener(ois_mouse, ois_keyboard)); 
    ois_mouse->setEventCallback(listener.get());
    ois_keyboard->setEventCallback(listener.get());
    Ogre::WindowEventUtilities::addWindowEventListener(ogre_window, listener.get());
    root->addFrameListener(listener.get());

    window_resized(window);
}

Controller::~Controller()
{
    ois_manager->destroyInputObject(ois_mouse);
	ois_manager->destroyInputObject(ois_keyboard);
    OIS::InputManager::destroyInputSystem(ois_manager);
}

void Controller::init()
{
	const std::string attribute_name {"WINDOW"};
	size_t window_handle;
    window->getCustomAttribute(attribute_name, &window_handle);

	OIS::ParamList ois_params;
	ois_params.insert(std::make_pair(attribute_name, std::to_string(window_handle)));
    
    #if defined OIS_LINUX_PLATFORM
    ois_params.insert(std::make_pair(std::string("x11_mouse_grab"),    std::string("false")));
    ois_params.insert(std::make_pair(std::string("x11_mouse_hide"),    std::string("false")));
    ois_params.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
    ois_params.insert(std::make_pair(std::string("XAutoRepeatOn"),     std::string("true")));
    #endif
    ois_manager = OIS::InputManager::createInputSystem(ois_params);

    ois_keyboard = static_cast<OIS::Keyboard*>(ois_manager->createInputObject(OIS::OISKeyboard, true));
	ois_mouse = static_cast<OIS::Mouse*>(ois_manager->createInputObject(OIS::OISMouse, true));
}

void Controller::window_resized(Ogre::RenderTarget* ogre_window)
{
    listener->window_resized(ogre_window);
}

void Controller::start()
{
  
    //what to do here? Do we need to spawn a new thread for listening on input?

}

bool Controller::register_input_listener(std::string id, ControllerBufferType* buffer)
{
    return listener->add_input_listener(id, buffer);
}

