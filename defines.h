#ifndef DEFINESH
#define DEFINESH

#define SCREENWIDTH   640
#define SCREENCENTERX 320.0
#define SCREENHEIGHT  320
#define SCREENCENTERY 160.0

#define RIGHT         true
#define LEFT          false

void func()
{
    asm
    {
        "%define SCREENWIDTH  640"
        "%define SCREENHEIGHT 320"
    }
}

#endif
