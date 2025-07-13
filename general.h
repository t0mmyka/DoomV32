#ifndef GENERALH
#define GENERALH

#include "defines.h"

union data
{
    int   in;
    float fl;
};

struct Player
{
    float xPos;
    float yPos;
    float zPos;
    float direction;
    float dirSin;
    float dirCos;
};


void inputWait()
{
    while(true)
    {
        if(gamepad_button_y() < 0)
        {
            return;
        }
        else if(gamepad_button_x() == 1)
        {
            end_frame();
            return;
        }
        end_frame();
    }
}


#endif
