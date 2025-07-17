#ifndef DEFINESH
#define DEFINESH

#define SCREENWIDTH   640
#define SCREENCENTERX 320.0
#define SCREENHEIGHT  320
#define SCREENCENTERY 160.0

#define RIGHT         true
#define LEFT          false

#define TAU     0x40c90fdb
#define PI3o2   0x4096cbe4
#define PIo2    0x3fc90fdb
#define PIo4    0x3f490fdb
#define PI3o4   0x4016cbe4

void func()
{
    asm
    {
        "%define SCREENWIDTH  640"
        "%define SCREENHEIGHT 320"
    }
}

#endif
