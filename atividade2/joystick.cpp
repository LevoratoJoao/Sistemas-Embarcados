#include "joystick.h"

void initJoystick(Joystick *joystick, int portX, int portY, int button)
{
    joystick->portX = portX;
    joystick->portY = portY;
    joystick->button = button;
}

void readJoystick(Joystick *joystick)
{
    
}