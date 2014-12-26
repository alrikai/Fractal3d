#include "InputListener.hpp"

bool InputListener::frameRenderingQueued (const Ogre::FrameEvent& frame_evt)
{
    mouse->capture();
    keyboard->capture();
    return true;
}

bool InputListener::frameStarted (const Ogre::FrameEvent& event) 
{
    //std::cout << "frame " << frame_count << " started" << std::endl;
    frame_count++;
    return true;
}

bool InputListener::frameEnded (const Ogre::FrameEvent& event)
{
    //std::cout << "frame " << frame_count << " ended" << std::endl;
    return true;
}

void InputListener::windowClosed (Ogre::RenderWindow* window)
{
    //do we need to do anything prior to the window being closed? -- this method is called 
    //when the user clicks the window "x" button, but hasn't actually been closed yet.
    //hence, this is the place to trigger any cleanup methods that require the renderwindow 
    //to still exist
 
    //...
    Ogre::Root::getSingletonPtr()->queueEndRendering();
}

bool InputListener::windowClosing(Ogre::RenderWindow* window)
{
    std::cout << "final window closing" << std::endl; 
    return true;
}
void InputListener::windowResized(Ogre::RenderWindow* window)
{
    window_resized(window);
}

void InputListener::window_resized(Ogre::RenderTarget* ogre_window)
{
    unsigned int width, height, depth; 
    ogre_window->getMetrics(width, height, depth);

    auto& mouse_state = mouse->getMouseState();
    mouse_state.width = width;
    mouse_state.height = height;    
}

bool InputListener::keyPressed (const OIS::KeyEvent& key_arg)
{
    current_key = key_arg.key;
	return true;
}

bool InputListener::keyReleased (const OIS::KeyEvent& key_arg)
{
    ControllerUtil::INPUT_TYPE evt;
    bool valid_key = true;
    auto key_type = key_arg.key;

    if (key_type == OIS::KC_ESCAPE)
        evt = ControllerUtil::INPUT_TYPE::Esc;	
    else if(key_arg.key == OIS::KC_UP)
        evt = ControllerUtil::INPUT_TYPE::UpArrow;
    else if(key_arg.key == OIS::KC_DOWN)
        evt = ControllerUtil::INPUT_TYPE::DArrow;
     else if(key_arg.key == OIS::KC_RIGHT)
        evt = ControllerUtil::INPUT_TYPE::RArrow;
     else if(key_arg.key == OIS::KC_LEFT)
        evt = ControllerUtil::INPUT_TYPE::LArrow;
     else if(key_arg.key == OIS::KC_A)
        evt = ControllerUtil::INPUT_TYPE::A;    
    else if(key_arg.key == OIS::KC_S)
        evt = ControllerUtil::INPUT_TYPE::S;    
    else if(key_arg.key == OIS::KC_W)
        evt = ControllerUtil::INPUT_TYPE::W;    
    else if(key_arg.key == OIS::KC_D)
        evt = ControllerUtil::INPUT_TYPE::D;
    else if(key_arg.key == OIS::KC_0)
        evt = ControllerUtil::INPUT_TYPE::ZERO;
    else if(key_arg.key == OIS::KC_1)
        evt = ControllerUtil::INPUT_TYPE::ONE;    
    else if(key_arg.key == OIS::KC_2)
        evt = ControllerUtil::INPUT_TYPE::TWO;    
    else if(key_arg.key == OIS::KC_3)
        evt = ControllerUtil::INPUT_TYPE::THREE;    
    else if(key_arg.key == OIS::KC_4)
        evt = ControllerUtil::INPUT_TYPE::FOUR;    
    else if(key_arg.key == OIS::KC_5)
        evt = ControllerUtil::INPUT_TYPE::FIVE;    
    else if(key_arg.key == OIS::KC_6)
        evt = ControllerUtil::INPUT_TYPE::SIX;    
    else if(key_arg.key == OIS::KC_7)
        evt = ControllerUtil::INPUT_TYPE::SEVEN;    
    else if(key_arg.key == OIS::KC_8)
        evt = ControllerUtil::INPUT_TYPE::EIGHT;    
    else if(key_arg.key == OIS::KC_9)
        evt = ControllerUtil::INPUT_TYPE::NINE;    
    else if(key_arg.key == OIS::KC_PGUP)
        evt = ControllerUtil::INPUT_TYPE::PUp;
    else if(key_arg.key == OIS::KC_PGDOWN)
        evt = ControllerUtil::INPUT_TYPE::PDown;
    else
        valid_key = false;

	if(valid_key)
  	    for (auto& m_listeners : input_listeners)
		    m_listeners.second->push(ControllerUtil::InputEvent(evt));

	return true;
}

bool InputListener::mouseMoved (const OIS::MouseEvent& mouse_arg)
{
    //track where the mouse is being dragged to
    if(mouse_dragging)
    {
        const OIS::MouseState &ms = mouse->getMouseState();
        ControllerUtil::INPUT_TYPE evt = ControllerUtil::INPUT_TYPE::MDrag;
    	for (auto& m_listeners : input_listeners)
		    m_listeners.second->push(ControllerUtil::InputEvent(evt, ms.X.abs - drag_pos_x, ms.Y.abs - drag_pos_y));          

        //update the last mouse position
        drag_pos_x = ms.X.abs;
        drag_pos_y = ms.Y.abs;
    }

	return true;
}

bool InputListener::mousePressed (const OIS::MouseEvent& mouse_arg, OIS::MouseButtonID mouse_id)
{
    mouse_dragging = true;
    const OIS::MouseState &ms = mouse->getMouseState();
    drag_pos_y = ms.Y.abs;
    drag_pos_x = ms.X.abs;
    return true;
}

bool InputListener::mouseReleased (const OIS::MouseEvent& mouse_arg, OIS::MouseButtonID mouse_id)
{
    ControllerUtil::INPUT_TYPE evt;
    bool valid_input = true;

    if(mouse_id == OIS::MouseButtonID::MB_Left)
        evt = ControllerUtil::INPUT_TYPE::LClick;
    else if(mouse_id == OIS::MouseButtonID::MB_Right)
        evt = ControllerUtil::INPUT_TYPE::RClick;
    else
        valid_input = false;
   
    if(valid_input)
    {
        const OIS::MouseState &ms = mouse->getMouseState();
    	for (auto& m_listeners : input_listeners)
		    m_listeners.second->push(ControllerUtil::InputEvent(evt, ms.X.abs, ms.Y.abs));      
    }
    mouse_dragging = false;
	return true;
}

bool InputListener::add_input_listener(std::string id, ControllerUtil::ControllerBufferType* buffer)
{
    return input_listeners.insert(std::make_pair(id, buffer)).second;;
}

