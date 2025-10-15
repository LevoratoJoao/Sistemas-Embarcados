#include "joystick.h"

#include <Arduino.h>

void setupJoystick(Joystick *joystick, int portX, int portY, int button)
{
    joystick->portX = portX;
    joystick->portY = portY;
    joystick->button = button;
}

bool isButtonPressed(Joystick *joystick)
{
    if (digitalRead(joystick->button) == LOW)
    {
        delay(5000);
        if (digitalRead(joystick->button) == LOW)
        {
            while (digitalRead(joystick->button) == LOW)
            {
                delay(10);
            }
            return true;
        }
    }
    return false;
}