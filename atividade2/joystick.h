#ifndef JOYSTICK_H
#define JOYSTICK_H

#define ANALOG_X_CORRECTION 128
#define ANALOG_Y_CORRECTION 128

typedef struct
{
    int portX;
    int portY;
    int button;
} Joystick;

void setupJoystick(Joystick *joystick, int portX, int portY, int button);
bool isButtonPressed(Joystick *joystick);

#endif