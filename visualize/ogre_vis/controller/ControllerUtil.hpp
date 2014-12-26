#ifndef TD_CONTROLLER_UTIL_HPP
#define TD_CONTROLLER_UTIL_HPP

#include <boost/lockfree/spsc_queue.hpp>

#include <iostream>

namespace ControllerUtil
{
    enum class INPUT_TYPE {NONE, A, S, W, D, LArrow, RArrow, UpArrow, DArrow, Esc, PUp, PDown, 
                           ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
                           LClick, RClick, MDrag};
    struct InputEvent
    {
        explicit InputEvent()
            : event_type(INPUT_TYPE::NONE), x_pos(-1), y_pos(-1)
        {}
        explicit InputEvent(INPUT_TYPE type)
            : event_type(type), x_pos(-1), y_pos(-1)
        {}
        InputEvent(INPUT_TYPE type, const int x, const int y)
            : event_type(type), x_pos(x), y_pos(y)
        {}

        INPUT_TYPE event_type;
        int x_pos;
        int y_pos;
    };

    typedef boost::lockfree::spsc_queue<InputEvent, boost::lockfree::capacity<1024>> ControllerBufferType;

    inline void print_input_type(INPUT_TYPE input)
    {
        switch(input)
        {
            case INPUT_TYPE::LArrow:
                std::cout << "Key L-Arrow" << std::endl;
            break;
            case INPUT_TYPE::RArrow:
                std::cout << "Key R-Arrow" << std::endl;
            break;
            case INPUT_TYPE::UpArrow:
                std::cout << "Key Up-Arrow" << std::endl;
            break;
            case INPUT_TYPE::DArrow:
                std::cout << "Key Down-Arrow" << std::endl;
            break;
            case INPUT_TYPE::PUp:
                std::cout << "Key Page-Up" << std::endl;
            break;
            case INPUT_TYPE::PDown:
                std::cout << "Key Page-Down" << std::endl;
            break;
            case INPUT_TYPE::A:
                std::cout << "Key A" << std::endl;
            break;
            case INPUT_TYPE::S:
                std::cout << "Key S" << std::endl;
            break;
            case INPUT_TYPE::W:
                std::cout << "Key W" << std::endl;
            break;
            case INPUT_TYPE::D:
                std::cout << "Key D" << std::endl;
            break;
            case INPUT_TYPE::ZERO:
                std::cout << "Key 0" << std::endl;
            break;
            case INPUT_TYPE::ONE:
                std::cout << "Key 1" << std::endl;
            break;
            case INPUT_TYPE::TWO:
                std::cout << "Key 2" << std::endl;
            break;
            case INPUT_TYPE::THREE:
                std::cout << "Key 3" << std::endl;
            break;
            case INPUT_TYPE::FOUR:
                std::cout << "Key 4" << std::endl;
            break;
            case INPUT_TYPE::FIVE:
                std::cout << "Key 5" << std::endl;
            break;
            case INPUT_TYPE::SIX:
                std::cout << "Key 6" << std::endl;
            break;
            case INPUT_TYPE::SEVEN:
                std::cout << "Key 7" << std::endl;
            break;
            case INPUT_TYPE::EIGHT:
                std::cout << "Key 8" << std::endl;
            break;
            case INPUT_TYPE::NINE:
                std::cout << "Key 9" << std::endl;
            break;
            case INPUT_TYPE::Esc:
                std::cout << "Key Escape" << std::endl;
            break;
            case INPUT_TYPE::LClick:
                std::cout << "Mouse Lclick" << std::endl;
            break;
            case INPUT_TYPE::RClick:
                std::cout << "Mouse Rclick" << std::endl;
            break;
            case INPUT_TYPE::MDrag:
                std::cout << "Mouse drag" << std::endl;
            break;            default:
                std::cout << "Unknown Type" << std::endl;
        };
    }

}

#endif
