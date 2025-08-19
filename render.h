#ifndef RENDERH
#define RENDERH

#include "defines.h"
#include "general.h"
#include "bsp.h"

struct FrameBuffer
{
    int[SCREENWIDTH]   fast;
    int[SCREENWIDTH*2] full; //(Bottom, Top)//
};

struct WallDrawData
{
    float        xStart;
    float        xEnd;
    float        yStart;
    float        yEnd;
    float        textureStart;
    float        textureEnd;
    float        zBottom;
    float        zTop;
    float        zPos;
    float        textureWidth;
    float        textureHeight;
    float        yOffset;
    FrameBuffer* clipping;
    int          floorColor;
    int          ceilingColor;
    int          textureID;
};

int mipMapGeometric(int width, int level)
{
    int add = width;
    int result = 0;

    while(level > 0)
    {
        result += add;
        add /= 2;
        level--;
    }

    return result;
}


void drawPortalClip(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zTop          = data->zTop;
    float        zBottom       = data->zBottom;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          floorColor    = data->floorColor;
    int          ceilingColor  = data->ceilingColor;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float bottomStart =  SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;


    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[columnStart]);


    select_texture(-1);
    set_drawing_scale(1.0, 1.0);
    select_region(256);

    asm
    {
        //Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_Cwall_while_loop_start:"

        //Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _Cwall_while_loop_iterators"

        //Set x drawing point
        "out  GPU_DrawingPointX, R6"

        //Get bottom clipping
        "mov  R5,  [R13]"
        "iadd R13, 1"

        //Get top clipping
        "mov  R4,  [R13]"
        "isub R13, 1"

        //Test if floor is clipped
        "mov  R0,  R9"
        "cfi  R0"
        "mov  R1,  R0"
        "igt  R1,  R5"
        "jt   R1,  _Cfloor_clipped"

        //Set new bottom clipping
        "imax R0,  R4"
        "mov  [R13], R0"

        //Get floor height
        "mov  R1,  R5"
        "isub R1,  R0"
        "cif  R1"

        //Draw floor
        "mov  R3,  {floorColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_Cfloor_clipped:"
        "iadd R13, 1"

        //Test if ceiling is clipped
        "mov  R0,  R10"
        "cfi  R0"
        "mov  R1,  R0"
        "ilt  R1,  R4"
        "jt   R1,  _Cceiling_clipped"

        //Set new top clipping
        "imin R0,  R5"
        "mov  [R13], R0"

        //Get ceiling height
        "mov  R1,  R0"
        "isub R1,  R4"
        "cif  R1"

        //Draw ceiling
        "mov  R3,  {ceilingColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R4"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"


        "_Cceiling_clipped:"

        //Test if column is full
        "mov  R0,  [R13]"
        "isub R13, 1"
        "mov  R1,  [R13]"
        "ige  R0,  R1"
        "jf   R0,  _Cwall_while_loop_iterators"

        "mov  [R7], R0"


        "_Cwall_while_loop_iterators:"

        //While iterators
        "iadd R6,  1"
        "iadd R7,  1"
        "isub R8,  1"
        "mov  R0,  {bottomStep}"
        "fadd R9,  R0"
        "mov  R0,  {topStep}"
        "fadd R10, R0"
        "iadd R13, 2"

        //Loop condition
        "jt   R8, _Cwall_while_loop_start"

        "_Cwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}




void drawPortalClipSkyBox(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zTop          = data->zTop;
    float        zBottom       = data->zBottom;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          floorColor    = data->floorColor;
    int          ceilingColor  = data->ceilingColor;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float bottomStart =  SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;


    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[columnStart]);


    select_texture(-1);
    set_drawing_scale(1.0, 1.0);
    select_region(256);

    asm
    {
        //Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_CSBwall_while_loop_start:"

        //Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _CSBwall_while_loop_iterators"

        //Set x drawing point
        "out  GPU_DrawingPointX, R6"

        //Get bottom clipping
        "mov  R5,  [R13]"
        "iadd R13, 1"

        //Get top clipping
        "mov  R4,  [R13]"
        "isub R13, 1"

        //Test if floor is clipped
        "mov  R0,  R9"
        "cfi  R0"
        "mov  R1,  R0"
        "igt  R1,  R5"
        "jt   R1,  _CSBfloor_clipped"

        //Set new bottom clipping
        "imax R0,  R4"
        "mov  [R13], R0"

        //Get floor height
        "mov  R1,  R5"
        "isub R1,  R0"
        "cif  R1"

        //Draw floor
        "mov  R3,  {floorColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_CSBfloor_clipped:"
        "iadd R13, 1"

        //Test if ceiling is clipped
        "mov  R0,  R10"
        "cfi  R0"
        "mov  R1,  R0"
        "ilt  R1,  R4"
        "jt   R1,  _CSBceiling_clipped"

        //Set new top clipping
        "imin R0,  R5"
        "mov  [R13], R0"

        //Get ceiling height
        "mov  R1,  R0"
        "isub R1,  R4"
        "cif  R1"

        //Draw ceiling
        "mov  R3,  {ceilingColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R4"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"


        "_CSBceiling_clipped:"

        //Set fast clipping
        "mov  R0,  1"
        "mov  [R7], R0"

        "isub R13, 1"

        "_CSBwall_while_loop_iterators:"

        //While iterators
        "iadd R6,  1"
        "iadd R7,  1"
        "isub R8,  1"
        "mov  R0,  {bottomStep}"
        "fadd R9,  R0"
        "mov  R0,  {topStep}"
        "fadd R10, R0"
        "iadd R13, 2"

        //Loop condition
        "jt   R8, _CSBwall_while_loop_start"

        "_CSBwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}




void drawPortalClipBottom(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zBottom       = data->zBottom;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          floorColor    = data->floorColor;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float bottomStart =  SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;


    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[columnStart]);


    select_texture(-1);
    set_drawing_scale(1.0, 1.0);
    select_region(256);

    asm
    {
        //Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R9,  {currentBottom}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_CBwall_while_loop_start:"

        //Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _CBwall_while_loop_iterators"

        //Set x drawing point
        "out  GPU_DrawingPointX, R6"

        //Get bottom clipping
        "mov  R5,  [R13]"
        "iadd R13, 1"

        //Get top clipping
        "mov  R4,  [R13]"
        "isub R13, 1"

        //Test if floor is clipped
        "mov  R0,  R9"
        "cfi  R0"
        "mov  R1,  R0"
        "igt  R1,  R5"
        "jt   R1,  _CBfloor_clipped"

        //Set new bottom clipping
        "imax R0,  R4"
        "mov  [R13], R0"

        //Get floor height
        "mov  R1,  R5"
        "isub R1,  R0"
        "cif  R1"

        //Draw floor
        "mov  R3,  {floorColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_CBfloor_clipped:"
        "iadd R13, 1"

        //Test if column is full
        "mov  R0,  [R13]"
        "isub R13, 1"
        "mov  R1,  [R13]"
        "ige  R0,  R1"
        "jf   R0,  _CBwall_while_loop_iterators"

        "mov  [R7], R0"


        "_CBwall_while_loop_iterators:"

        //While iterators
        "iadd R6,  1"
        "iadd R7,  1"
        "isub R8,  1"
        "mov  R0,  {bottomStep}"
        "fadd R9,  R0"
        "iadd R13, 2"

        //Loop condition
        "jt   R8, _CBwall_while_loop_start"

        "_CBwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}



void drawPortalClipBottomSkyBox(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zBottom       = data->zBottom;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          floorColor    = data->floorColor;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float bottomStart =  SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
                      - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;


    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[columnStart]);


    select_texture(-1);
    set_drawing_scale(1.0, 1.0);
    select_region(256);

    asm
    {
        //Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R9,  {currentBottom}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_CBSBwall_while_loop_start:"

        //Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _CBSBwall_while_loop_iterators"

        //Set x drawing point
        "out  GPU_DrawingPointX, R6"

        //Get bottom clipping
        "mov  R5,  [R13]"
        "iadd R13, 1"

        //Get top clipping
        "mov  R4,  [R13]"
        "isub R13, 1"

        //Test if floor is clipped
        "mov  R0,  R9"
        "cfi  R0"
        "mov  R1,  R0"
        "igt  R1,  R5"
        "jt   R1,  _CBSBfloor_clipped"

        //Set new bottom clipping
        "imax R0,  R4"
        "mov  [R13], R0"

        //Get floor height
        "mov  R1,  R5"
        "isub R1,  R0"
        "cif  R1"

        //Draw floor
        "mov  R3,  {floorColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_CBSBfloor_clipped:"
        "_CBSBwall_while_loop_iterators:"

        //While iterators
        "iadd R6,  1"
        "mov  R0,  1"
        "mov  [R7], R0"
        "iadd R7,  1"
        "isub R8,  1"
        "mov  R0,  {bottomStep}"
        "fadd R9,  R0"
        "iadd R13, 2"

        //Loop condition
        "jt   R8, _CBSBwall_while_loop_start"

        "_CBSBwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}




void drawPortalClipTop(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zTop          = data->zTop;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          ceilingColor  = data->ceilingColor;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[columnStart]);


    select_texture(-1);
    set_drawing_scale(1.0, 1.0);
    select_region(256);

    asm
    {
        //Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R10, {currentTop}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_CTwall_while_loop_start:"

        //Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _CTwall_while_loop_iterators"

        //Set x drawing point
        "out  GPU_DrawingPointX, R6"

        //Get bottom clipping
        "mov  R5,  [R13]"
        "iadd R13, 1"

        //Get top clipping
        "mov  R4,  [R13]"

        //Test if ceiling is clipped
        "mov  R0,  R10"
        "cfi  R0"
        "mov  R1,  R0"
        "ilt  R1,  R4"
        "jt   R1,  _CTceiling_clipped"

        //Set new top clipping
        "imin R0,  R5"
        "mov  [R13], R0"

        //Get ceiling height
        "mov  R1,  R0"
        "isub R1,  R4"
        "cif  R1"

        //Draw ceiling
        "mov  R3,  {ceilingColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R1"
        "out  GPU_DrawingPointY, R4"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"


        "_CTceiling_clipped:"

        //Test if column is full
        "mov  R0,  [R13]"
        "isub R13, 1"
        "mov  R1,  [R13]"
        "ige  R0,  R1"
        "jf   R0,  _CTwall_while_loop_iterators"

        "mov  [R7], R0"


        "_CTwall_while_loop_iterators:"

        //While iterators
        "iadd R6,  1"
        "iadd R7,  1"
        "isub R8,  1"
        "mov  R0,  {topStep}"
        "fadd R10, R0"
        "iadd R13, 2"

        //Loop condition
        "jt   R8, _CTwall_while_loop_start"

        "_CTwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}


void drawPortalClipTopSkyBox(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zTop          = data->zTop;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          ceilingColor  = data->ceilingColor;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
                      - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[columnStart]);


    select_texture(-1);
    set_drawing_scale(1.0, 1.0);
    select_region(256);

    asm
    {
        //Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R10, {currentTop}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_CTSBwall_while_loop_start:"

        //Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _CTSBwall_while_loop_iterators"

        //Set x drawing point
        "out  GPU_DrawingPointX, R6"

        //Get bottom clipping
        "mov  R5,  [R13]"
        "iadd R13, 1"

        //Get top clipping
        "mov  R4,  [R13]"

        //Test if ceiling is clipped
        "mov  R0,  R10"
        "cfi  R0"
        "mov  R1,  R0"
        "ilt  R1,  R4"
        "jt   R1,  _CTSBceiling_clipped"

        //Get ceiling height
        "imin R0,  R5"
        "isub R0,  R4"
        "cif  R0"

        //Draw ceiling
        "mov  R3,  {ceilingColor}"
        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingScaleY, R0"
        "out  GPU_DrawingPointY, R4"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"




        "_CTSBceiling_clipped:"
        "isub R13, 1"

        "_CTSBwall_while_loop_iterators:"

        //While iterators
        "iadd R6,  1"
        "mov  R0,  1"
        "mov  [R7], R0"
        "iadd R7,  1"
        "isub R8,  1"
        "mov  R0,  {topStep}"
        "fadd R10, R0"
        "iadd R13, 2"

        //Loop condition
        "jt   R8, _CTSBwall_while_loop_start"

        "_CTSBwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}



void drawPortal(
    WallDrawData* data,
    float windowBottomZ,
    float windowTopZ,
    Texture* upperTexture
)
{
    float xStart              = data->xStart;
    float xEnd                = data->xEnd;
    float yStart              = data->yStart;
    float yEnd                = data->yEnd;
    float textureStart        = data->textureStart;
    float textureEnd          = data->textureEnd;
    float zBottom             = data->zBottom;
    float zTop                = data->zTop;
    float zPos                = data->zPos;
    float textureWidthBottom  = data->textureWidth;
    float textureHeightBottom = data->textureHeight;
    float textureWidthTop     = upperTexture->width;
    float textureHeightTop    = upperTexture->height;
    float yOffset             = data->yOffset;
    int*  fastClipping        = data->clipping->fast;
    int*  fullClipping        = data->clipping->full;
    float roomHeight          = zTop - zBottom;
    float topWallHeight       = zTop - windowTopZ;
    float bottomWallHeight    = windowBottomZ - zBottom;
    int   floorColor          = data->floorColor;
    int   ceilingColor        = data->ceilingColor;
    int   textureBottom       = data->textureID;
    int   textureTop          = upperTexture->textureID;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth <= 0.0)
    {
        return;
    }

    float topStart          =  SCREENCENTERY
                            - (SCREENHEIGHT * (zTop          - zPos) / xStart);

    float topEnd            = SCREENCENTERY
                            - (SCREENHEIGHT * (zTop          - zPos) / xEnd);

    float windowTopStart    =  SCREENCENTERY
                            - (SCREENHEIGHT * (windowTopZ    - zPos) / xStart);

    float windowTopEnd      = SCREENCENTERY
                            - (SCREENHEIGHT * (windowTopZ    - zPos) / xEnd);

    float bottomStart       =  SCREENCENTERY
                            - (SCREENHEIGHT * (zBottom       - zPos) / xStart);

    float bottomEnd         = SCREENCENTERY
                            - (SCREENHEIGHT * (zBottom       - zPos) / xEnd);

    float windowBottomStart =  SCREENCENTERY
                            - (SCREENHEIGHT * (windowBottomZ - zPos) / xStart);

    float windowBottomEnd   = SCREENCENTERY
                            - (SCREENHEIGHT * (windowBottomZ - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentWindowTop = windowTopStart;
    float windowTopStep    = (windowTopEnd - windowTopStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float currentWindowBottom = windowBottomStart;
    float windowBottomStep    = (windowBottomEnd - windowBottomStart) / onScreenWidth;

    float currentTextureScale = (bottomStart - topStart) / roomHeight;

    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
                              / (roomHeight * onScreenWidth);

    float texOverXstart = textureStart / xStart;
    float texOverXend   = textureEnd   / xEnd;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / xStart;
    float inverseXend   = 1.0 / xEnd;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    float textureTrueTop          = textureHeightTop - yOffset - roomHeight;
    float textureTrueWindowTop    = textureTrueTop + topWallHeight;
    float textureTrueBottom       = textureHeightBottom - yOffset;
    float textureTrueWindowBottom = textureTrueBottom - bottomWallHeight;
    int*  fullClippingPointer = &(fullClipping[columnStart*2]);
    int*  fastClippingPointer = &(fastClipping[0]);
    int   screenTop;
    int   screenBottom;
    int   screenFullTop;
    int   screenFullBottom;
    int   textureFullTop;
    int   textureFullBottom;

    select_texture(textureBottom);
    select_region(0);
    set_multiply_color(0xFFFFFFFF);
    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentWindowBottom}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_Pwall_while_loop_start:"

        //Check fast clipping
        "mov  R1,  {fastClippingPointer}"
        "iadd R1,  R11"
        "mov  R0,  [R1]"
        "jt   R0,  _Pwall_while_loop_iterators_add"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _Ptop_offscreen"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R2,  [R13]"
        "cif  R2"
        "flt  R0,  R2"
        "jt   R0,  _Pbottom_offsecreen"

        //Set X drawing point
        "out  GPU_DrawingPointX,  R11"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidthBottom}"
        "fmod R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _Pbottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueBottom}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _Pbottom_clipped_end"



        //Bottom is not clipped
        "_Pbottom_not_clipped:"

        //Select floor settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R1,  {floorColor}"
        "out  GPU_MultiplyColor, R1"

        //Get floor height
        "mov  R1,  R12"
        "cif  R1"
        "fsub R1,  R9"
        "ceil R1"
        "out  GPU_DrawingScaleY, R1"

        //Draw floor
        "mov  R1,  R9"
        "cfi  R1"
        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureBottom}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueBottom}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_Pbottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _Ptop_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueWindowBottom}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _Ptop_clipped_end"



        //Top is not clipped
        "_Ptop_not_clipped:"

        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueWindowBottom}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_Ptop_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _PfullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PfullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _PtopTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PtopTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _PbottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PbottomTextureSkip:"


        // Set full clipping
        "isub R13, 1"
        "mov  R12, {screenTop}"
        "mov  [R13], R12"
        "iadd R13, 2"

        "jmp _Pwall_while_loop_iterators"

        // Go to next full clipping
        "_Pwall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp _Pwall_while_loop_iterators"

        // Set fast clipping true
        "_Pbottom_offsecreen:"
        "mov  [R1], R0"

        // Go to next full clipping
        "_Ptop_offscreen:"
        "iadd R13, 1"


        "_Pwall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {windowBottomStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _Pwall_while_loop_start"

        "_Pwall_while_loop_end:"
    }

    select_texture(textureTop);
    select_region(0);
    set_multiply_color(0xFFFFFFFF);
    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentWindowTop}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_PTwall_while_loop_start:"

        //Check fast clipping
        "mov  R1,   {fastClippingPointer}"
        "iadd R1,   R11"
        "mov  R0,   [R1]"
        "jt   R0,   _PTwall_while_loop_iterators_add"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _PTtop_offscreen"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R1,  [R13]"
        "cif  R1"
        "flt  R0,  R1"
        "jt   R0,  _PTbottom_offsecreen"

        //Set X drawing point
        "out  GPU_DrawingPointX,  R11"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidthTop}"
        "fmod R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _PTbottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueWindowTop}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _PTbottom_clipped_end"



        //Bottom is not clipped
        "_PTbottom_not_clipped:"

        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueWindowTop}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_PTbottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _PTtop_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueTop}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _PTtop_clipped_end"



        //Top is not clipped
        "_PTtop_not_clipped:"

        //Select ceiling settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {ceilingColor}"
        "out  GPU_MultiplyColor, R2"

        //Get ceiling height
        "mov  R2,  R10"
        "fsub R2,  R1"
        "flr  R2"
        "out  GPU_DrawingScaleY, R2"

        //Draw ceiling
        "mov  R2,  R1"
        "cfi  R2"
        "out  GPU_DrawingPointY, R2"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureTop}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueTop}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_PTtop_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _PTfullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PTfullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _PTtopTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PTtopTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _PTbottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PTbottomTextureSkip:"


        // Set full clipping
        "mov  R12, {screenBottom}"
        "mov  [R13], R12"
        "iadd R13, 1"

        "jmp  _PTwall_while_loop_iterators"

        // Go to next full clipping
        "_PTwall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp  _PTwall_while_loop_iterators"


        // Set fast clipping true
        "_PTtop_offscreen:"
        "mov  [R1], R0"

        // Go to next full clipping
        "_PTbottom_offsecreen:"
        "iadd R13, 1"


        "_PTwall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {windowTopStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _PTwall_while_loop_start"

        "_PTwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}



void drawPortalSkyBox(
    WallDrawData* data,
    float windowBottomZ,
    float windowTopZ,
    Texture* upperTexture
)
{
    float xStart              = data->xStart;
    float xEnd                = data->xEnd;
    float yStart              = data->yStart;
    float yEnd                = data->yEnd;
    float textureStart        = data->textureStart;
    float textureEnd          = data->textureEnd;
    float zBottom             = data->zBottom;
    float zTop                = data->zTop;
    float zPos                = data->zPos;
    float textureWidthBottom  = data->textureWidth;
    float textureHeightBottom = data->textureHeight;
    float textureWidthTop     = upperTexture->width;
    float textureHeightTop    = upperTexture->height;
    float yOffset             = data->yOffset;
    int*  fastClipping        = data->clipping->fast;
    int*  fullClipping        = data->clipping->full;
    float roomHeight          = zTop - zBottom;
    float topWallHeight       = zTop - windowTopZ;
    float bottomWallHeight    = windowBottomZ - zBottom;
    int   floorColor          = data->floorColor;
    int   ceilingColor        = data->ceilingColor;
    int   textureBottom       = data->textureID;
    int   textureTop          = upperTexture->textureID;

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth <= 0.0)
    {
        return;
    }

    float topStart          =  SCREENCENTERY
                            - (SCREENHEIGHT * (zTop          - zPos) / xStart);

    float topEnd            = SCREENCENTERY
                            - (SCREENHEIGHT * (zTop          - zPos) / xEnd);

    float windowTopStart    =  SCREENCENTERY
                            - (SCREENHEIGHT * (windowTopZ    - zPos) / xStart);

    float windowTopEnd      = SCREENCENTERY
                            - (SCREENHEIGHT * (windowTopZ    - zPos) / xEnd);

    float bottomStart       =  SCREENCENTERY
                            - (SCREENHEIGHT * (zBottom       - zPos) / xStart);

    float bottomEnd         = SCREENCENTERY
                            - (SCREENHEIGHT * (zBottom       - zPos) / xEnd);

    float windowBottomStart =  SCREENCENTERY
                            - (SCREENHEIGHT * (windowBottomZ - zPos) / xStart);

    float windowBottomEnd   = SCREENCENTERY
                            - (SCREENHEIGHT * (windowBottomZ - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentWindowTop = windowTopStart;
    float windowTopStep    = (windowTopEnd - windowTopStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float currentWindowBottom = windowBottomStart;
    float windowBottomStep    = (windowBottomEnd - windowBottomStart) / onScreenWidth;

    float currentTextureScale = (bottomStart - topStart) / roomHeight;

    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
                              / (roomHeight * onScreenWidth);

    float texOverXstart = textureStart / xStart;
    float texOverXend   = textureEnd   / xEnd;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / xStart;
    float inverseXend   = 1.0 / xEnd;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    float textureTrueTop          = textureHeightTop - yOffset - roomHeight;
    float textureTrueWindowTop    = textureTrueTop + topWallHeight;
    float textureTrueBottom       = textureHeightBottom - yOffset;
    float textureTrueWindowBottom = textureTrueBottom - bottomWallHeight;
    int*  fullClippingPointer = &(fullClipping[columnStart*2]);
    int*  fastClippingPointer = &(fastClipping[0]);
    int   screenTop;
    int   screenBottom;
    int   screenFullTop;
    int   screenFullBottom;
    int   textureFullTop;
    int   textureFullBottom;

    select_texture(textureBottom);
    select_region(0);
    set_multiply_color(0xFFFFFFFF);
    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentWindowBottom}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_PSBBwall_while_loop_start:"

        //Check fast clipping
        "mov  R1,  {fastClippingPointer}"
        "iadd R1,  R11"
        "mov  R0,  [R1]"
        "jt   R0,  _PSBBwall_while_loop_iterators_add"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _PSBBtop_offscreen"

        //Set X drawing point
        "out  GPU_DrawingPointX,  R11"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R2,  [R13]"
        "cif  R2"
        "flt  R0,  R2"
        "jt   R0,  _PSBBbottom_offsecreen"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidthBottom}"
        "fmod R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _PSBBbottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueBottom}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _PSBBbottom_clipped_end"



        //Bottom is not clipped
        "_PSBBbottom_not_clipped:"

        //Select floor settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R1,  {floorColor}"
        "out  GPU_MultiplyColor, R1"

        //Get floor height
        "mov  R1,  R12"
        "cif  R1"
        "fsub R1,  R9"
        "ceil R1"
        "out  GPU_DrawingScaleY, R1"

        //Draw floor
        "mov  R1,  R9"
        "cfi  R1"
        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureBottom}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueBottom}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_PSBBbottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _PSBBtop_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueWindowBottom}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _PSBBtop_clipped_end"



        //Top is not clipped
        "_PSBBtop_not_clipped:"

        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueWindowBottom}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_PSBBtop_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _PSBBfullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PSBBfullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _PSBBtopTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PSBBtopTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _PSBBbottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PSBBbottomTextureSkip:"


        "iadd R13, 1"

        "jmp _PSBBwall_while_loop_iterators"

        // Go to next full clipping
        "_PSBBwall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp _PSBBwall_while_loop_iterators"

        // Set fast clipping true
        "_PSBBbottom_offsecreen:"

        //Select floor settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R1,  {floorColor}"
        "out  GPU_MultiplyColor, R1"

        //Get floor height
        "mov  R1,  R12"
        "cif  R1"
        "fsub R1,  R2"
        "ceil R1"
        "out  GPU_DrawingScaleY, R1"

        //Draw floor
        "mov  R1,  R2"
        "cfi  R1"
        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureBottom}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // Go to next full clipping
        "_PSBBtop_offscreen:"
        "iadd R13, 1"


        "_PSBBwall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {windowBottomStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _PSBBwall_while_loop_start"

        "_PSBBwall_while_loop_end:"
    }

    select_texture(textureTop);
    select_region(0);
    set_multiply_color(0xFFFFFFFF);
    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentWindowTop}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_PSBTwall_while_loop_start:"

        //Check fast clipping
        "mov  R1,   {fastClippingPointer}"
        "iadd R1,   R11"
        "mov  R0,   [R1]"
        "jt   R0,   _PSBTwall_while_loop_iterators_add"

        //Set fast clipping true
        "mov  R0,  1"
        "mov  [R1], R0"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Set X drawing point
        "out  GPU_DrawingPointX,  R11"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _PSBTtop_offscreen"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R1,  [R13]"
        "cif  R1"
        "flt  R0,  R1"
        "jt   R0,  _PSBTbottom_offsecreen"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidthTop}"
        "fmod R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _PSBTbottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueWindowTop}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _PSBTbottom_clipped_end"



        //Bottom is not clipped
        "_PSBTbottom_not_clipped:"

        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueWindowTop}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_PSBTbottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _PSBTtop_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueTop}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _PSBTtop_clipped_end"



        //Top is not clipped
        "_PSBTtop_not_clipped:"

        //Select ceiling settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {ceilingColor}"
        "out  GPU_MultiplyColor, R2"

        //Get ceiling height
        "mov  R2,  R10"
        "fsub R2,  R1"
        "flr  R2"
        "out  GPU_DrawingScaleY, R2"

        //Draw ceiling
        "mov  R2,  R1"
        "cfi  R2"
        "out  GPU_DrawingPointY, R2"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureTop}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueTop}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_PSBTtop_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _PSBTfullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PSBTfullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _PSBTtopTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PSBTtopTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _PSBTbottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_PSBTbottomTextureSkip:"


        "iadd R13, 1"

        "jmp  _PSBTwall_while_loop_iterators"

        // Go to next full clipping
        "_PSBTwall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp  _PSBTwall_while_loop_iterators"


        // Set fast clipping true
        "_PSBTtop_offscreen:"
        "mov  [R1], R0"

        //Select ceiling settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {ceilingColor}"
        "out  GPU_MultiplyColor, R2"

        //Get ceiling height
        "mov  R1,  [R13]"
        "isub R12, R1"
        "cif  R12"
        "out  GPU_DrawingScaleY, R12"

        //Draw ceiling
        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureTop}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // Go to next full clipping
        "_PSBTbottom_offsecreen:"
        "iadd R13, 1"


        "_PSBTwall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {windowTopStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _PSBTwall_while_loop_start"

        "_PSBTwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}




void drawPortalBottom(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        textureStart  = data->textureStart;
    float        textureEnd    = data->textureEnd;
    float        zBottom       = data->zBottom;
    float        zTop          = data->zTop;
    float        zPos          = data->zPos;
    float        textureWidth  = data->textureWidth;
    float        textureHeight = data->textureHeight;
    float        yOffset       = data->yOffset;
    FrameBuffer* clipping      = data->clipping;
    float        roomHeight    = zTop - zBottom;
    int          floorColor    = data->floorColor;
    int          textureNUM    = data->textureID;


    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
    - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
    - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float bottomStart =  SCREENCENTERY
    - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
    - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float currentTextureScale = (bottomStart - topStart) / roomHeight;

    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
    / (roomHeight * onScreenWidth);

    float texOverXstart = textureStart / xStart;
    float texOverXend   = textureEnd   / xEnd;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / xStart;
    float inverseXend   = 1.0 / xEnd;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    float textureTrueTop      = textureHeight - yOffset - roomHeight;
    float textureTrueBottom   = textureHeight - yOffset;
    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[0]);
    int   screenTop;
    int   screenBottom;
    int   screenFullTop;
    int   screenFullBottom;
    int   textureFullTop;
    int   textureFullBottom;


    select_texture(textureNUM);
    select_region(0);

    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_Bwall_while_loop_start:"

        //Check fast clipping
        "mov  R1,  {fastClippingPointer}"
        "iadd R1,  R11"
        "mov  R0,  [R1]"
        "jt   R0,  _Bwall_while_loop_iterators_add"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _Btop_offscreen"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R2,  [R13]"
        "cif  R2"
        "flt  R0,  R2"
        "jt   R0,  _Bbottom_offsecreen"

        //Set X drawing point
        "out  GPU_DrawingPointX,  R11"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidth}"
        "fmod R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _Bbottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueBottom}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _Bbottom_clipped_end"



        //Bottom is not clipped
        "_Bbottom_not_clipped:"

        //Select floor settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R1,  {floorColor}"
        "out  GPU_MultiplyColor, R1"

        //Get floor height
        "mov  R1,  R12"
        "cif  R1"
        "fsub R1,  R9"
        "ceil R1"
        "out  GPU_DrawingScaleY, R1"

        //Draw floor
        "mov  R1,  R9"
        "cfi  R1"
        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureNUM}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueBottom}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_Bbottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _Btop_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueTop}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _Btop_clipped_end"



        //Top is not clipped
        "_Btop_not_clipped:"

        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueTop}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_Btop_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _BfullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_BfullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _BtopTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_BtopTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _BbottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_BbottomTextureSkip:"


        // Set full clipping
        "isub R13, 1"
        "mov  R12, {screenTop}"
        "mov  [R13], R12"
        "iadd R13, 2"

        "jmp _Bwall_while_loop_iterators"

        // Go to next full clipping
        "_Bwall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp _Bwall_while_loop_iterators"

        // Set fast clipping true
        "_Bbottom_offsecreen:"
        "mov  [R1], R0"

        // Go to next full clipping
        "_Btop_offscreen:"
        "iadd R13, 1"


        "_Bwall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _Bwall_while_loop_start"

        "_Bwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}





void drawPortalTop(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        textureStart  = data->textureStart;
    float        textureEnd    = data->textureEnd;
    float        zBottom       = data->zBottom;
    float        zTop          = data->zTop;
    float        zPos          = data->zPos;
    float        textureWidth  = data->textureWidth;
    float        textureHeight = data->textureHeight;
    float        yOffset       = data->yOffset;
    FrameBuffer* clipping      = data->clipping;
    float        roomHeight    = zTop - zBottom;
    int          ceilingColor  = data->ceilingColor;
    int          textureNUM    = data->textureID;


    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart + 1);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
    - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
    - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float bottomStart =  SCREENCENTERY
    - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
    - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float currentTextureScale = (bottomStart - topStart) / roomHeight;

    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
    / (roomHeight * onScreenWidth);

    float texOverXstart = textureStart / xStart;
    float texOverXend   = textureEnd   / xEnd;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / xStart;
    float inverseXend   = 1.0 / xEnd;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    float textureTrueTop      = textureHeight - yOffset - roomHeight;
    float textureTrueBottom   = textureHeight - yOffset;
    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[0]);
    int   screenTop;
    int   screenBottom;
    int   screenFullTop;
    int   screenFullBottom;
    int   textureFullTop;
    int   textureFullBottom;


    select_texture(textureNUM);
    select_region(0);

    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_Twall_while_loop_start:"

        //Check fast clipping
        "mov  R1,   {fastClippingPointer}"
        "iadd R1,   R11"
        "mov  R0,   [R1]"
        "jt   R0,   _Twall_while_loop_iterators_add"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _Ttop_offscreen"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R1,  [R13]"
        "cif  R1"
        "flt  R0,  R1"
        "jt   R0,  _Tbottom_offsecreen"

        //Set X drawing point
        "out  GPU_DrawingPointX,  R11"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidth}"
        "fmod R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _Tbottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueBottom}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _Tbottom_clipped_end"



        //Bottom is not clipped
        "_Tbottom_not_clipped:"

        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueBottom}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_Tbottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _Ttop_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueTop}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _Ttop_clipped_end"



        //Top is not clipped
        "_Ttop_not_clipped:"

        //Select ceiling settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {ceilingColor}"
        "out  GPU_MultiplyColor, R2"

        //Get ceiling height
        "mov  R2,  R10"
        "fsub R2,  R1"
        "flr  R2"
        "out  GPU_DrawingScaleY, R2"

        //Draw ceiling
        "mov  R2,  R1"
        "cfi  R2"
        "out  GPU_DrawingPointY, R2"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureNUM}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"


        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueTop}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_Ttop_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _TfullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_TfullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _TtopTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_TtopTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _TbottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_TbottomTextureSkip:"


        // Set full clipping
        "mov  R12, {screenBottom}"
        "mov  [R13], R12"
        "iadd R13, 1"

        "jmp  _Twall_while_loop_iterators"

        // Go to next full clipping
        "_Twall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp  _Twall_while_loop_iterators"


        // Set fast clipping true
        "_Ttop_offscreen:"
        "mov  [R1], R0"

        // Go to next full clipping
        "_Tbottom_offsecreen:"
        "iadd R13, 1"


        "_Twall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _Twall_while_loop_start"

        "_Twall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}





void drawWall(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        textureStart  = data->textureStart;
    float        textureEnd    = data->textureEnd;
    float        zBottom       = data->zBottom;
    float        zTop          = data->zTop;
    float        zPos          = data->zPos;
    float        textureWidth  = data->textureWidth;
    float        textureHeight = data->textureHeight;
    float        yOffset       = data->yOffset;
    FrameBuffer* clipping      = data->clipping;
    float        roomHeight    = zTop - zBottom;
    int          floorColor    = data->floorColor;
    int          ceilingColor  = data->ceilingColor;
    int          textureNUM    = data->textureID;

    if(xStart == 0.0 || yStart == 0.0 || xEnd == 0.0 || yEnd == 0.0)
    {
        return;
    }

    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 639);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 639);

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    =  SCREENCENTERY
    - (SCREENHEIGHT * (zTop    - zPos) / xStart);

    float topEnd      = SCREENCENTERY
    - (SCREENHEIGHT * (zTop    - zPos) / xEnd);

    float bottomStart =  SCREENCENTERY
    - (SCREENHEIGHT * (zBottom - zPos) / xStart);

    float bottomEnd   = SCREENCENTERY
    - (SCREENHEIGHT * (zBottom - zPos) / xEnd);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float currentTextureScale = (bottomStart - topStart) / roomHeight;
    float inverseMaxScale = 5.0 * roomHeight
                          / fmax(bottomStart - topStart, bottomEnd - topEnd);

    //int mipMapLevel = max(floor(log(inverseMaxScale) / LN2), 0.0);
    int mipMapLevel = 0.0;

    float mipMapStart = mipMapGeometric(textureWidth, mipMapLevel);

    int mipMapFactor = pow(2.0, (float)mipMapLevel);

    textureWidth /= (float)mipMapFactor;
    textureStart /= (float)mipMapFactor;
    textureEnd   /= (float)mipMapFactor;

    currentTextureScale *= (float)mipMapFactor;

    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
                              / (roomHeight * onScreenWidth);

    float texOverXstart = textureStart / xStart;
    float texOverXend   = textureEnd   / xEnd;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / xStart;
    float inverseXend   = 1.0 / xEnd;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;


    float textureTrueTop      = textureHeight
                              - ((yOffset + roomHeight) / mipMapFactor);
    float textureTrueBottom   = textureHeight
                              - (yOffset / mipMapFactor);

    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[0]);
    int   screenTop;
    int   screenBottom;
    int   screenFullTop;
    int   screenFullBottom;
    int   textureFullTop;
    int   textureFullBottom;

    select_texture(textureNUM);
    set_drawing_scale(1.0, 1.0);
    select_region(0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "iadd R7,  1"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_wall_while_loop_start:"

        //Check fast clipping
        "mov  R1,   {fastClippingPointer}"
        "iadd R1,   R11"
        "mov  R0,   [R1]"
        "jt   R0,   _wall_while_loop_iterators_add"
        "mov  R0,   1"
        "mov  [R1], R0"

        //Set X drawing point
        "out  GPU_DrawingPointX, R11"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is below screen
        "mov  R0,  R10"
        "mov  R3,  R12"
        "cif  R3"
        "fgt  R0,  R3"
        "jt   R0,  _top_offscreen"

        //Check if bottom is above screen
        "mov  R0,  R9"
        "mov  R1,  [R13]"
        "cif  R1"
        "flt  R0,  R1"
        "jt   R0,  _bottom_offsecreen"

        //Determine currentTextureX
        "mov  R0,  R4"
        "fdiv R0,  R5"

        "mov  R1,  {textureWidth}"
        "fmod R0,  R1"
        "mov  R1,  {mipMapStart}"
        "fadd R0,  R1"

        //Set X positions
        "cfi  R0"
        "out  GPU_RegionMinX, R0"
        "out  GPU_RegionMaxX, R0"
        "out  GPU_RegionHotspotX, R0"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "fgt  R0,  R3"
        "jf   R0,  _bottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R12"

        // textureBottom
        "mov  R0,  {textureTrueBottom}"
        "mov  R2,  R9"
        "cif  R12"
        "fsub R2,  R12"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R12, R0"
        "cfi  R12"
        "mov  {screenFullBottom}, R12"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _bottom_clipped_end"



        //Bottom is not clipped
        "_bottom_not_clipped:"

        //Select floor settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {floorColor}"
        "out  GPU_MultiplyColor, R2"

        //Get floor height
        "mov  R2,  R12"
        "cif  R2"
        "fsub R2,  R9"
        "ceil R2"
        "out  GPU_DrawingScaleY, R2"

        //Draw floor
        "mov  R2,  R9"
        "cfi  R2"
        "out  GPU_DrawingPointY, R2"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureNUM}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"

        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueBottom}"

        // textureFullBottom
        "mov  R2,  R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_bottom_clipped_end:"


        //Get top clipping
        "mov  R12, [R13]"
        "iadd R13, 1"

        //Check if top is clipped
        "mov  R0,  R10"
        "mov  R1,  R12"
        "cif  R1"
        "flt  R0,  R1"
        "jf   R0,  _top_not_clipped"

        //Top is clipped

        // screenTop
        "mov  {screenTop}, R12"

        // textureTop
        "mov  R0,  {textureTrueTop}"
        "mov  R2,  R10"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullTop
        "mov  R2,  R0"
        "ceil R2"

        // screenFullTop
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "jmp  _top_clipped_end"



        //Top is not clipped
        "_top_not_clipped:"

        //Select ceiling settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {ceilingColor}"
        "out  GPU_MultiplyColor, R2"

        //Get ceiling height
        "mov  R2,  R10"
        "fsub R2,  R1"
        "flr  R2"
        "out  GPU_DrawingScaleY, R2"

        //Draw ceiling
        "mov  R2,  R1"
        "cfi  R2"
        "out  GPU_DrawingPointY, R2"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureNUM}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"

        // screenTop
        "mov  R1,  R10"
        "cfi  R1"
        "mov  {screenTop}, R1"
        "mov  R1,  R10"

        // textureTop
        "mov  R0, {textureTrueTop}"

        // textureFullTop
        "mov  R2, R0"
        "ceil R2"

        // screenFullTop
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullTop}, R1"

        "cfi  R2"
        "mov  {textureFullTop}, R2"

        "_top_clipped_end:"




        //Texture Full Check
        "mov  R2, {textureFullBottom}"
        "mov  R3, {textureFullTop}"
        "ile  R2, R3"
        "jt   R2, _fullTextureSkip"

        //Texture Full drawing
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenFullTop}"
        "mov  R2,  {textureFullBottom}"
        "mov  R3,  {textureFullTop}"
        "isub R0,  R1"
        "isub R2,  R3"
        "cif  R0"
        "cif  R2"
        "fdiv R0,  R2"
        "out  GPU_DrawingScaleY, R0"

        "mov  R2,  {textureFullBottom}"
        "out  GPU_RegionMinY, R3"
        "out  GPU_RegionHotspotY, R2"
        "isub R2,  1"
        "out  GPU_RegionMaxY, R2"

        "mov  R0,  {screenFullBottom}"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_fullTextureSkip:"

        //Texture Top Check
        "mov  R0,  {screenFullTop}"
        "mov  R1,  {screenTop}"
        "ine  R1,  R0"
        "jf   R1,  _topTextureSkip"

        //Texture Top Drawing
        "mov  R1,  {screenBottom}"
        "imin R0,  R1"
        "mov  R1,  {screenTop}"
        "isub R0,  R1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  {textureFullTop}"
        "isub R0,  1"
        "out  GPU_RegionMinY, R0"
        "out  GPU_RegionMaxY, R0"
        "out  GPU_RegionHotspotY, R0"

        "out  GPU_DrawingPointY, R1"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_topTextureSkip:"

        //Texture Bottom Check
        "mov  R0,  {screenFullBottom}"
        "mov  R1,  {screenBottom}"
        "ine  R1,  R0"
        "jf   R1,  _bottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R1,  {screenTop}"
        "imax R0,  R1"
        "mov  R1,  {screenBottom}"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY, R1"

        "mov  R1,  {textureFullBottom}"
        "out  GPU_RegionMinY, R1"
        "out  GPU_RegionMaxY, R1"
        "out  GPU_RegionHotspotY, R1"

        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_bottomTextureSkip:"

        "jmp _wall_while_loop_iterators"


        // Go to next full clipping
        "_wall_while_loop_iterators_add:"

        "iadd R13, 2"

        "jmp _wall_while_loop_iterators"


        "_bottom_offsecreen:"

        //Select floor settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {floorColor}"
        "out  GPU_MultiplyColor, R2"

        //Get floor height
        "fsub R1,  R3"
        "ceil R1"
        "out  GPU_DrawingScaleY, R1"

        //Draw floor
        "isub R12,  1"
        "out  GPU_DrawingPointY, R12"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureNUM}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"

        "jmp  _offscreen_end"

        "_top_offscreen:"

        //Select ceiling settings
        "in   R0,  GPU_SelectedTexture"
        "out  GPU_SelectedTexture, -1"
        "out  GPU_SelectedRegion, 256"
        "mov  R2,  {ceilingColor}"
        "out  GPU_MultiplyColor, R2"

        //Get ceiling height
        "mov  R1,  [R13]"
        "cif  R1"
        "fsub R1,  R3"
        "ceil R1"
        "out  GPU_DrawingScaleY, R1"

        //Draw ceiling
        "isub R12,  1"
        "out  GPU_DrawingPointY, R12"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        //Reset settings
        "out  GPU_MultiplyColor, 0xFFFFFFFF"
        "mov  R0,  {textureNUM}"
        "out  GPU_SelectedTexture, R0"
        "out  GPU_SelectedRegion, 0"

        // Go to next full clipping
        "_offscreen_end:"
        "iadd R13, 1"



        "_wall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        //Check loop condition
        "jt   R7,  _wall_while_loop_start"

        "_wall_while_loop_end:"

    }

    //set_multiply_color(color_white);

    return;
}





void drawSegment(FrameBuffer* clipping, Segment* seg, Player* pov)
{
    float povX = pov->xPos;
    float povY = pov->yPos;

    data segStartX;
    data segEndX;

    data segStartY;
    data segEndY;

    float segDeltaX;
    float segDeltaY;

    bool backFace = false;

    if(!onRightSideSeg(seg, povX, povY)) //Backface culling
    {
        if(seg->isPortal)
        {
            segEndX.fl = pov->dirCos * (seg->xPos - povX)
                        + pov->dirSin * (seg->yPos - povY);

            segDeltaX  = pov->dirCos * seg->dx
                       + pov->dirSin * seg->dy;

            segStartX.fl   = segEndX.fl + segDeltaX;

            if((segStartX.in & segEndX.in) < 0) //Behind culling
            {
                return;
            }


            segEndY.fl = pov->dirCos * (seg->yPos - povY)
                       - pov->dirSin * (seg->xPos - povX);

            segDeltaY  = pov->dirCos * seg->dy
                       - pov->dirSin * seg->dx;

            segStartY.fl = segEndY.fl + segDeltaY;

            backFace = true;
        }
        else
        {
            return;
        }
    }
    else
    {
        segStartX.fl = pov->dirCos * (seg->xPos - povX)
                     + pov->dirSin * (seg->yPos - povY);

        segDeltaX    = pov->dirCos * seg->dx
                     + pov->dirSin * seg->dy;

        segEndX.fl   = segStartX.fl + segDeltaX;

        if((segStartX.in & segEndX.in) < 0) //Behind culling
        {
            return;
        }


        segStartY.fl = pov->dirCos * (seg->yPos - povY)
                     - pov->dirSin * (seg->xPos - povX);

        segDeltaY    = pov->dirCos * seg->dy
                     - pov->dirSin * seg->dx;

        segEndY.fl   = segStartY.fl + segDeltaY;
    }

    data temp;
    temp.in = segStartX.in | 0x80000000;
    if(segStartY.fl < temp.fl) //Start point culling
    {
        return;
    }

    temp.in = segEndX.in & 0x7FFFFFFF;
    if(segEndY.fl > temp.fl) //End point culling
    {
        return;
    }

    float drawStartX;
    float drawStartY;
    float drawEndX;
    float drawEndY;

    float textureXstart;
    float textureXend;

    if(segStartY.fl > segStartX.fl) //Start Value adjustment needed
    {
        drawStartY    = (segStartX.fl * segDeltaY - segStartY.fl * segDeltaX)
                      / (segDeltaY - segDeltaX);
        drawStartX    = drawStartY;
        textureXstart = seg->xOffset + sqrt(
                        (segStartX.fl - drawStartX)*(segStartX.fl - drawStartX)
                      + (segStartY.fl - drawStartY)*(segStartY.fl - drawStartY));
    }
    else
    {
        drawStartX    = segStartX.fl;
        drawStartY    = segStartY.fl;
        textureXstart = seg->xOffset;
    }


    if(segEndY.fl < segEndX.fl * -1.0) //End value adjustment needed
    {
        drawEndY    = ((segStartY.fl * segDeltaX) - (segStartX.fl * segDeltaY))
                    / (segDeltaX + segDeltaY);
        drawEndX    = -1.0 * drawEndY;
        textureXend = (seg->length - sqrt(
                      (segEndX.fl - drawEndX)*(segEndX.fl - drawEndX)
                    + (segEndY.fl - drawEndY)*(segEndY.fl - drawEndY)))
                    + seg->xOffset;
    }
    else
    {
        drawEndX    = segEndX.fl;
        drawEndY    = segEndY.fl;
        textureXend = seg->xOffset + seg->length;
    }
    WallDrawData drawData;

    if(backFace == true) //BACKFACE
    {
        if(seg->isSkyBox == false)
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = seg->sectorRight->floorHeight;
            drawData.zBottom        = seg->sectorLeft->floorHeight;
            drawData.zTop           = seg->sectorLeft->floorHeight;
            drawData.zTop           = seg->sectorLeft->ceilingHeight;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->bottom->width;
            drawData.textureHeight  = seg->bottom->height;
            drawData.yOffset        = seg->yOffset;
            drawData.clipping       = clipping;
            drawData.floorColor     = seg->sectorLeft->floorColor;
            drawData.ceilingColor   = seg->sectorLeft->ceilingColor;

            drawPortalClip(&drawData);
        }
    }
    else if(seg->isPortal == false) //SOLID WALL
    {
        if(seg->isSkyBox == false) //NORMAL WALL
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = seg->sectorRight->floorHeight;
            drawData.zTop           = seg->sectorRight->ceilingHeight;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->middle->width;
            drawData.textureHeight  = seg->middle->height;
            drawData.yOffset        = seg->yOffset;
            drawData.clipping       = clipping;
            drawData.floorColor     = seg->sectorRight->floorColor;
            drawData.ceilingColor   = seg->sectorRight->ceilingColor;
            drawData.textureID      = seg->middle->textureID;

            drawWall(&drawData);
        }
        else //SKYBOX WALL
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = seg->sectorRight->floorHeight;
            drawData.zTop           = seg->sectorRight->ceilingHeight;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->middle->width;
            drawData.textureHeight  = seg->middle->height;
            drawData.yOffset        = seg->yOffset;
            drawData.clipping       = clipping;
            drawData.floorColor     = seg->sectorRight->floorColor;
            drawData.ceilingColor   = seg->sectorRight->ceilingColor;
            drawData.textureID      = seg->middle->textureID;

            drawPortalClipSkyBox(&drawData);
        }
    }
    else //PORTAL`
    {
        float floorHeightFront   = seg->sectorRight->floorHeight;
        float floorHeightBack    = seg->sectorLeft->floorHeight;
        float ceilingHeightFront = seg->sectorRight->ceilingHeight;
        float ceilingHeightBack  = seg->sectorLeft->ceilingHeight;

        if(
            floorHeightFront >= floorHeightBack &&
            ceilingHeightFront <= ceilingHeightBack
        )
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = floorHeightFront;
            drawData.zTop           = ceilingHeightFront;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->bottom->width;
            drawData.textureHeight  = seg->bottom->height;
            drawData.yOffset        = seg->yOffset;
            drawData.clipping       = clipping;
            drawData.textureID      = seg->bottom->textureID;
            drawData.floorColor     = seg->sectorRight->floorColor;
            drawData.ceilingColor   = seg->sectorRight->ceilingColor;

            if(seg->isSkyBox == false)
            {
                drawPortalClip(&drawData);
            }
            else
            {
                drawPortalClipSkyBox(&drawData);
            }
        }
        else if (floorHeightFront >= floorHeightBack)
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = ceilingHeightBack;
            drawData.zTop           = ceilingHeightFront;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->bottom->width;
            drawData.textureHeight  = seg->bottom->height;
            drawData.yOffset        = seg->yOffset
                                    + ceilingHeightBack
                                    - floorHeightFront;
            drawData.clipping       = clipping;
            drawData.textureID      = seg->top->textureID;
            drawData.floorColor     = seg->sectorRight->floorColor;
            drawData.ceilingColor   = seg->sectorRight->ceilingColor;

            drawPortalTop(&drawData);

            drawData.zBottom        = floorHeightFront;

            if(seg->isSkyBox == false)
            {
                drawPortalClipBottom(&drawData);
            }
            else
            {
                drawPortalClipBottomSkyBox(&drawData);
            }
        }
        else if(ceilingHeightFront <= ceilingHeightBack)
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = floorHeightFront;
            drawData.zTop           = floorHeightBack;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->bottom->width;
            drawData.textureHeight  = seg->bottom->height;
            drawData.yOffset        = seg->yOffset;
            drawData.clipping       = clipping;
            drawData.textureID      = seg->bottom->textureID;
            drawData.floorColor     = seg->sectorRight->floorColor;
            drawData.ceilingColor   = seg->sectorRight->ceilingColor;

            drawPortalBottom(&drawData);

            drawData.zTop           = ceilingHeightFront;

            if(seg->isSkyBox == false)
            {
                drawPortalClipTop(&drawData);
            }
            else
            {
                drawPortalClipTopSkyBox(&drawData);
            }
        }
        else
        {
            drawData.xStart         = drawStartX;
            drawData.xEnd           = drawEndX;
            drawData.yStart         = drawStartY;
            drawData.yEnd           = drawEndY;
            drawData.textureStart   = textureXstart;
            drawData.textureEnd     = textureXend;
            drawData.zBottom        = floorHeightFront;
            drawData.zTop           = ceilingHeightFront;
            drawData.zPos           = pov->zPos + pov->camZ;
            drawData.textureWidth   = seg->bottom->width;
            drawData.textureHeight  = seg->bottom->height;
            drawData.yOffset        = seg->yOffset;
            drawData.clipping       = clipping;
            drawData.textureID      = seg->bottom->textureID;
            drawData.floorColor     = seg->sectorRight->floorColor;
            drawData.ceilingColor   = seg->sectorRight->ceilingColor;

            if(seg->isSkyBox == false)
            {
                drawPortal(
                    &drawData,
                    seg->sectorLeft->floorHeight,
                    seg->sectorLeft->ceilingHeight,
                    seg->top
                );
            }
            else
            {
                drawPortalSkyBox(
                    &drawData,
                    seg->sectorLeft->floorHeight,
                    seg->sectorLeft->ceilingHeight,
                    seg->top
                );
            }
        }
    }

    return;
}


void drawBspLeaf(FrameBuffer* clipping, BspLeaf* leaf, Player* pov)
{
    int      segSize = sizeof(Segment*);
    Segment** segList = leaf->segList;
    while(*segList != NULL)
    {
        drawSegment(clipping, *segList, pov);
        segList += segSize;
    }

    return;
}


void bspRender(int* filledFast, FrameBuffer* clipping, BspBranch* currentNode, Player* pov)
{
    BspBranch* tempBranch;
    bool  side;
    bool[2] sideVisability;
    float xPos = pov->xPos;
    float yPos = pov->yPos;

    if(currentNode->leaf != NULL)
    {
        drawBspLeaf(clipping, currentNode->leaf, pov);
        if(memcmp(filledFast, clipping->fast, SCREENWIDTH) == 0)
            return;
    }

    side = onRightSideBranch(currentNode, xPos, yPos);
    areBranchesVisable(currentNode, pov, sideVisability);

    if(side == RIGHT)
    {
        if(sideVisability[1] == true)
        {
            tempBranch = currentNode->rightNode;
            if(tempBranch != NULL)
                bspRender(filledFast, clipping, tempBranch, pov);

            if(memcmp(filledFast, clipping->fast, SCREENWIDTH) == 0)
                return;
        }

        if(sideVisability[0] == true)
        {
            tempBranch = currentNode->leftNode;
            if(currentNode->leftNode != NULL)
                bspRender(filledFast, clipping, tempBranch,  pov);
        }
    }
    else
    {
        if(sideVisability[0] == true)
        {
            tempBranch = currentNode->leftNode;
            if(currentNode->leftNode != NULL)
                bspRender(filledFast, clipping, tempBranch,  pov);

            if(memcmp(filledFast, clipping->fast, SCREENWIDTH) == 0)
                return;
        }

        if(sideVisability[1] == true)
        {
            tempBranch = currentNode->rightNode;
            if(tempBranch != NULL)
                bspRender(filledFast, clipping, tempBranch, pov);
        }
    }

    return;
}

void drawSkyBox(SkyBox* sky, Player* pov)
{
    Texture* skyTexture = sky->texture;
    select_texture(skyTexture->textureID);

    float xScale = SCREENWIDTH / (skyTexture->width / 4.0);
    float yScale = (float)SCREENHEIGHT / (float)(skyTexture->height);

    set_drawing_scale(xScale, yScale);

    float startAngle = PIo4 + pov->direction - sky->rotation;
    float endAngle   = startAngle - PIo2;

    float startTexture = (1.0 - fmod(fmod((startAngle / TAU), 1.0) + 1.0, 1.0))
                       * skyTexture->width;

    float endTexture = (1.0 - fmod(fmod((endAngle / TAU), 1.0) + 1.0, 1.0))
                     * skyTexture->width;

    if(startTexture < endTexture) //No seam
    {
        select_region(0);
        define_region(
            floor(startTexture),
            0,
            ceil(endTexture),
            skyTexture->height,
            floor(startTexture),
            0
        );

        draw_region_zoomed_at((floor(startTexture) - startTexture) * xScale, 0);
    }
    else //Seam
    {
        select_region(0);
        define_region(
            floor(startTexture),
            0,
            skyTexture->width,
            skyTexture->height,
            floor(startTexture),
            0
        );

        draw_region_zoomed_at((floor(startTexture) - startTexture) * xScale, 0);

        define_region(
            0,
            0,
            ceil(endTexture),
            skyTexture->height,
            0,
            0
        );

        draw_region_zoomed_at((skyTexture->width - startTexture) * xScale, 0);
    }

    return;
}


#endif
