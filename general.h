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
    float xSpeed;
    float ySpeed;
    float zSpeed;
    float direction;
    float dirSin;
    float dirCos;
    float height;
    float camZ;
};

struct Ray
{
    float xPos;
    float yPos;
    float zPos;
    float dx;
    float dy;
    float dz;
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

float dist(float a, float b)
{
    float distance;

    asm
    {
        "mov  R0, {a}"
        "mov  R1, {b}"
        "fmul R0, R0"
        "fmul R1, R1"
        "fadd R0, R1"
        "mov  R1, 0.5"
        "pow  R0, R1"
        "mov  {distance}, R0"
    }

    return distance;
}

float distSQRD(float a, float b)
{
    return (a * a) + (b * b);
}

void normalize(float* x, float* y)
{
    float angle = atan2(*y, *x);
    *x = cos(angle);
    *y = sin(angle);
}

bool isNegativeF(float num)
{
    asm
    {
        "mov  R0, {num}"
        "shl  R0, -31"
        "mov {num}, R0"
    }
    return num;
}

float nextafter(float from, float to)
{
    float result = from;

    if(to > from)
    {
        if(isNegativeF(from))
        {
            if(from == -0.0)
            {
                asm
                {
                    "mov  R0, 1"
                    "mov {result}, R0"
                }
                return result;
            }
            else
            {
                asm
                {
                    "mov  R0, {result}"
                    "isub R0, 1"
                    "mov {result}, R0"
                }
                return result;
            }
        }
        else
        {
            asm
            {
                "mov  R0, {result}"
                "iadd R0, 1"
                "mov {result}, R0"
            }
            return result;
        }
    }
    else if(to < from)
    {
        if(isNegativeF(from))
        {
            asm
            {
                "mov  R0, {result}"
                "isub R0, 1"
                "mov {result}, R0"
            }
        }
        else
        {
            if(from == 0.0)
            {
                asm
                {
                    "mov  R0, 0x80000001"
                    "mov {result}, R0"
                }
                return result;
            }
            else
            {
                asm
                {
                    "mov  R0, {result}"
                    "iadd R0, 1"
                    "mov {result}, R0"
                }
                return result;
            }
        }
    }
    else
    {
        return from;
    }
}

float fClamp(float input, float min, float max)
{
    asm
    {
        "mov  R0, {input}"
        "mov  R1, {min}"
        "mov  R2, {max}"
        "fmax R0, R1"
        "fmin R0, R2"
        "mov  {input}, R0"
    }
    return input;
}

#endif
