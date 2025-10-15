#include "joystick.h"

#include <Arduino.h>

void setupJoystick(Joystick *joystick, int portX, int portY, int button)
{
    joystick->portX = portX;
    joystick->portY = portY;
    joystick->button = button;
    joystick->lastButtonTime = 0;
    joystick->lastJoystickTime = 0;
    joystick->lastButtonState = HIGH;
}

bool isButtonPressed(Joystick *joystick)
{
    bool currentState = digitalRead(joystick->button) == LOW;
    unsigned long currentTime = millis();

    if (currentState != joystick->lastButtonState &&
        currentTime - joystick->lastButtonTime > 50)
    {
        joystick->lastButtonTime = currentTime;
        joystick->lastButtonState = currentState;
        return currentState;
    }
    return false;
}

JoystickDirection isJoystickMoved(Joystick *joystick)
{
    unsigned long currentTime = millis();
    if (currentTime - joystick->lastJoystickTime < 300)
        return NONE;

    int yValue = analogRead(joystick->portY);
    if (yValue >= 900)
    {
        joystick->lastJoystickTime = currentTime;
        return UP;
    }
    else if (yValue <= 100)
    {
        joystick->lastJoystickTime = currentTime;
        return DOWN;
    }
    int xValue = analogRead(joystick->portX);
    if (xValue >= 900)
    {
        joystick->lastJoystickTime = currentTime;
        return RIGHT;
    }
    else if (xValue <= 100)
    {
        joystick->lastJoystickTime = currentTime;
        return LEFT;
    }
    return NONE;
}

// TODO: esc para sair de editar