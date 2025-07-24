#ifndef DEFINESH
#define DEFINESH

#define SCREENWIDTH   640
#define SCREENCENTERX 320.0
#define SCREENHEIGHT  320
#define SCREENCENTERY 160.0

#define RIGHT         true
#define LEFT          false

#define TAU     6.283185482025146484375
#define PI3o2   4.7123889923095703125
#define PIo2    1.57079637050628662109375
#define PI3o4   2.35619449615478515625
#define PIo4    0.785398185253143310546875

#define LN2     0.693147182464599609375

void func()
{
    asm
    {
        "%define SCREENWIDTH  640"
        "%define SCREENHEIGHT 320"
    }
}

#endif
