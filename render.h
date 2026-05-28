#ifndef RENDERH
#define RENDERH

#include "defines.h"
#include "general.h"
#include "bsp.h"

struct FrameBuffer
{
    int[SCREENWIDTH] data;
};

struct WallSpanData
{
    float xStart;
    float xEnd;
    float yStart;
    float yEnd;
    float textureStart;
    float textureEnd;
    bool  backFace;
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
    int          depthID;
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






void drawBillboard(Entity *billboard, Entity *pov)
{
    Texture* sprite;

    float posX = pov->dirCos * (billboard->xPos - pov->xPos)
               + pov->dirSin * (billboard->yPos - pov->yPos);

    float posY = pov->dirCos * (billboard->yPos - pov->yPos)
               - pov->dirSin * (billboard->xPos - pov->xPos);

    float posZ = billboard->zPos - pov->zPos - pov->camZ;

    if(posX <= 0)
        return;


    sprite = &billboard->sprites[0];

    float scale = (billboard->height / posX) / 2.0 * SCREENRATIO;
          scale = scale * SCREENHEIGHT / sprite->height;

    float screenX = (1.0 - (posY/posX)) / 2.0;
          screenX = SCREENWIDTH * screenX + SCREENXPOS;
    float screenY = (1.0 - (posZ/posX)) / 2.0 * SCREENRATIO;
          screenY = SCREENHEIGHT * screenY + SCREENYPOS;

    if(screenX + (scale * sprite->width / 2.0) < SCREENXPOS)
        return;

    if(screenX - (scale * sprite->width / 2.0) > SCREENXPOS + SCREENWIDTH)
        return;

    set_multiply_color(color_white);
    select_texture(sprite->textureID);
    select_region(0);
    define_region(0, 0, sprite->width, sprite->height, sprite->width/2, sprite->height);
    set_drawing_scale(scale, scale);

    draw_region_zoomed_at(screenX, screenY);
}






bool convertSegment(WallSpanData* span, Segment *seg, Entity *pov)
{
    float povX = pov->xPos;
    float povY = pov->yPos;

    data segStartX;
    data segEndX;

    data segStartY;
    data segEndY;

    float segDeltaX;
    float segDeltaY;

    span->backFace = false;

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
                return false;
            }


            segEndY.fl = pov->dirCos * (seg->yPos - povY)
                       - pov->dirSin * (seg->xPos - povX);

            segDeltaY  = pov->dirCos * seg->dy
                       - pov->dirSin * seg->dx;

            segStartY.fl = segEndY.fl + segDeltaY;

            span->backFace = true;
        }
        else
        {
            return false;
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
            return false;
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
        return false;
    }

    temp.in = segEndX.in & 0x7FFFFFFF;
    if(segEndY.fl > temp.fl) //End point culling
    {
        return false;
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


    span->xStart       = drawStartX;
    span->xEnd         = drawEndX;
    span->yStart       = drawStartY;
    span->yEnd         = drawEndY;
    span->textureStart = textureXstart;
    span->textureEnd   = textureXend;

    return true;
}

void clipWall(WallDrawData* data, int xPos, int width)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    FrameBuffer* clipping      = data->clipping;
    int          depthID       = data->depthID;

    if(xStart == 0.0 || yStart == 0.0 || xEnd == 0.0 || yEnd == 0.0)
    {
        return;
    }

    int   columnStart  = min(floor(width * (1.0 - yStart/xStart) / 2.0), width);
          columnStart += xPos;
    int   columnEnd    = min(floor(width * (1.0 - yEnd/xEnd) / 2.0), width);
          columnEnd   += xPos;

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }


    int* depthMapPointer = &(clipping->data[columnStart - xPos]);

    asm
    {
        "mov  R3,  {onScreenWidth}"
        "cfi  R3"
        "mov  R4,  {depthMapPointer}"
        "mov  R5,  {depthID}"

        "_depth_fill_start:"

        "mov  R0,  [R4]"
        "ine  R0,  0"
        "jt   R0,  _depth_fill_iterators"

        "mov  [R4], R5"

        "_depth_fill_iterators:"

        "iadd R4,  1"
        "isub R3,  1"

        "jt   R3,  _depth_fill_start"
    }

    return;
}






void clipSegment(FrameBuffer* clipping, Segment* seg, Entity* pov, int ID)
{
    WallSpanData spanData;
    WallDrawData drawData;

    if(!convertSegment(&spanData, seg, pov))
    {
        return;
    }

    drawData.xStart   = spanData.xStart;
    drawData.xEnd     = spanData.xEnd;
    drawData.yStart   = spanData.yStart;
    drawData.yEnd     = spanData.yEnd;
    drawData.clipping = clipping;
    drawData.depthID  = ID;

    clipWall(&drawData, SCREENXPOS, SCREENWIDTH);
}






bool drawWall(WallDrawData* data, int xPos, int yPos, int width, int height)
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
    int          textureNUM    = data->textureID;
    int          depthID       = data->depthID;

    if(roomHeight <= 0)
    {
        return false;
    }

    if(xStart == 0.0 || yStart == 0.0 || xEnd == 0.0 || yEnd == 0.0)
    {
        return false;
    }

    int   columnStart  = min(floor(width * (1.0 - yStart/xStart) / 2.0), width);
          columnStart += xPos;
    int   columnEnd    = min(floor(width * (1.0 - yEnd/xEnd) / 2.0), width);
          columnEnd   += xPos;

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return false;
    }

    float topStart    = ((1.0 - SCREENRATIO * (zTop - zPos) / xStart) / 2.0);
          topStart    = (float)height * topStart + (float)yPos;

    float topEnd      = ((1.0 - SCREENRATIO * (zTop - zPos) / xEnd) / 2.0);
          topEnd      = (float)height * topEnd + (float)yPos;

    float bottomStart = ((1.0 - SCREENRATIO * (zBottom - zPos) / xStart) / 2.0);
          bottomStart = (float)height * bottomStart + (float)yPos;

    float bottomEnd   = ((1.0 - SCREENRATIO * (zBottom - zPos) / xEnd) / 2.0);
          bottomEnd   = (float)height * bottomEnd + (float)yPos;


    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float texOverXstart = textureStart / xStart;
    float texOverXend   = textureEnd   / xEnd;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / xStart;
    float inverseXend   = 1.0 / xEnd;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    int textureTrueBottom = round(textureHeight - yOffset) - 1;
    int textureTrueTop    = textureTrueBottom - max(round(roomHeight), 1) + 1;

    int pixelHeight = textureTrueBottom - textureTrueTop + 1;

    int*  depthMapPointer   = &(clipping->data[columnStart - xPos]);

    bool  seen = false;

    set_multiply_color(color_white);
    select_texture(textureNUM);
    set_drawing_scale(1.0, 1.0);
    select_region(0);
    define_region(0, textureTrueTop, 0, textureTrueBottom, 0, textureTrueBottom + 1);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "mov  R8,  {pixelHeight}"
        "cif  R8"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R12, {depthID}"
        "mov  R13, {depthMapPointer}"

        //Loop start
        "_wall_while_loop_start:"

        //Check fast clipping
        "mov  R0,   [R13]"
        "ilt  R0,   R12"
        "jt   R0,   _wall_while_loop_iterators"

        //Set X drawing point
        "out  GPU_DrawingPointX, R11"

        //SEEN
        "mov  R0,  1"
        "mov  {seen}, R0"

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

        //Texture Full drawing
        "mov  R0,  R9"
        "cfi  R0"
        "mov  R1,  R10"
        "cfi  R1"
        "isub R0,  R1"
        "cif  R0"
        "fdiv R0,  R8"
        "out  GPU_DrawingScaleY, R0"

        "mov  R0,  R9"
        "cfi  R0"
        "out  GPU_DrawingPointY, R0"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_wall_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "isub R7,  1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "iadd R11, 1"

        "iadd R13, 1"

        //Check loop condition
        "jt   R7,  _wall_while_loop_start"

        "_wall_while_loop_end:"
    }

    return seen;
}






void drawPlanes(WallDrawData* data, int xPos, int yPos, int width, int height)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zBottom       = data->zBottom;
    float        zTop          = data->zTop;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          floorColor    = data->floorColor;
    int          ceilingColor  = data->ceilingColor;
    int          depthID       = data->depthID;

    if(xStart == 0.0 || yStart == 0.0 || xEnd == 0.0 || yEnd == 0.0)
    {
        return;
    }

    int   columnStart  = min(floor(width * (1.0 - yStart/xStart) / 2.0), width);
          columnStart += xPos;
    int   columnEnd    = min(floor(width * (1.0 - yEnd/xEnd) / 2.0), width);
          columnEnd   += xPos;

    float onScreenWidth = (float)(columnEnd - columnStart);
    if(onScreenWidth == 0.0)
    {
        return;
    }

    float topStart    = ((1.0 - SCREENRATIO * (zTop - zPos) / xStart) / 2.0);
          topStart    = (float)height * topStart + (float)yPos;

    float topEnd      = ((1.0 - SCREENRATIO * (zTop - zPos) / xEnd) / 2.0);
          topEnd      = (float)height * topEnd + (float)yPos;

    float bottomStart = ((1.0 - SCREENRATIO * (zBottom - zPos) / xStart) / 2.0);
          bottomStart = (float)height * bottomStart + (float)yPos;

    float bottomEnd   = ((1.0 - SCREENRATIO * (zBottom - zPos) / xEnd) / 2.0);
          bottomEnd   = (float)height * bottomEnd + (float)yPos;


    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    int*  depthMapPointer   = &(clipping->data[columnStart - xPos]);

    select_texture(-1);
    select_region(256);
    set_drawing_scale(1.0, 1.0);

    asm
    {
        //Initialize registers
        "mov  R3,  {ceilingColor}"
        "mov  R4,  {floorColor}"
        "mov  R5,  {yPos}"
        "mov  R6,  {height}"
        "iadd R6,  R5"
        "mov  R7,  {onScreenWidth}"
        "cfi  R7"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R11, {columnStart}"
        "mov  R12, {depthID}"
        "mov  R13, {depthMapPointer}"

        //Loop start
        "_plane_while_loop_start:"

        //Check fast clipping
        "mov  R0,   [R13]"
        "ilt  R0,   R12"
        "jt   R0,   _plane_while_loop_iterators"

        //Set X drawing point
        "out  GPU_DrawingPointX, R11"

        //Ceiling drawing
        "mov  R0,  R10"
        "cfi  R0"
        "mov  R1,  R5"
        "igt  R1,  R0"
        "jt   R1,  _plane_ceiling_done"
        "isub R0,  R5"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "out  GPU_MultiplyColor, R3"
        "out  GPU_DrawingPointY, R5"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_plane_ceiling_done:"

        //Floor drawing
        "mov  R0,  R9"
        "cfi  R0"
        "mov  R1,  R6"
        "ilt  R1,  R0"
        "jt   R1,  _plane_floor_done"
        "out  GPU_DrawingPointY, R0"
        "isub R0,  R6"
        "imul R0,  -1"
        "cif  R0"
        "out  GPU_DrawingScaleY, R0"

        "out  GPU_MultiplyColor, R4"
        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        "_plane_floor_done:"

        "_plane_while_loop_iterators:"

        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "isub R7,  1"

        "iadd R11, 1"

        "iadd R13, 1"

        //Check loop condition
        "jt   R7,  _plane_while_loop_start"

        "_plane_while_loop_end:"
    }

    return;
}






void drawSegment(FrameBuffer* clipping, Segment* seg, Entity* pov, int ID)
{
    WallSpanData spanData;
    WallDrawData drawData;

    if(!convertSegment(&spanData, seg, pov))
        return;

    drawData.xStart       = spanData.xStart;
    drawData.xEnd         = spanData.xEnd;
    drawData.yStart       = spanData.yStart;
    drawData.yEnd         = spanData.yEnd;
    drawData.zPos         = pov->zPos + pov->camZ;
    drawData.textureStart = spanData.textureStart;
    drawData.textureEnd   = spanData.textureEnd;
    drawData.clipping     = clipping;
    drawData.depthID      = ID;

    if(spanData.backFace == true) //BACKFACE
    {
        if(seg->isSkyBox == false) //BACK OF PORTAL
        {
            drawData.zBottom      = seg->sectorRight->floorHeight;
            drawData.zBottom      = seg->sectorLeft->floorHeight;
            drawData.zTop         = seg->sectorLeft->floorHeight;
            drawData.zTop         = seg->sectorLeft->ceilingHeight;
            drawData.floorColor   = seg->sectorLeft->floorColor;
            drawData.ceilingColor = seg->sectorLeft->ceilingColor;

            drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
        }
    }
    else if(seg->isPortal == false) //SOLID WALL
    {
        drawData.floorColor   = seg->sectorRight->floorColor;
        drawData.ceilingColor = seg->sectorRight->ceilingColor;
        drawData.zBottom      = seg->sectorRight->floorHeight;
        drawData.zTop         = seg->sectorRight->ceilingHeight;

        if(seg->isSkyBox == false) //NORMAL WALL
        {
            drawData.textureWidth  = seg->middle->width;
            drawData.textureHeight = seg->middle->height;
            drawData.yOffset       = seg->yOffset;
            drawData.textureID     = seg->middle->textureID;

            seg->seen |= drawWall(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
            drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
        }
        else //SKYBOX WALL (planes only)
        {
            seg->seen = true;
            drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
        }
    }
    else //PORTAL`
    {
        drawData.floorColor   = seg->sectorRight->floorColor;
        drawData.ceilingColor = seg->sectorRight->ceilingColor;

        float floorHeightFront   = seg->sectorRight->floorHeight;
        float floorHeightBack    = seg->sectorLeft->floorHeight;
        float ceilingHeightFront = seg->sectorRight->ceilingHeight;
        float ceilingHeightBack  = seg->sectorLeft->ceilingHeight;

        if(
            floorHeightFront >= floorHeightBack &&
            ceilingHeightFront <= ceilingHeightBack
        ) //CANT SEE (planes only)
        {
            drawData.zBottom      = floorHeightFront;
            drawData.zTop         = ceilingHeightFront;

            seg->seen = true;

            if(seg->isSkyBox == false)
            {
                drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
            }
            else
            {
                drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
            }
        }
        else if (floorHeightFront >= floorHeightBack) //ONLY TOP (and planes)
        {
            drawData.zBottom       = ceilingHeightBack;
            drawData.zTop          = ceilingHeightFront;
            drawData.textureWidth  = seg->bottom->width;
            drawData.textureHeight = seg->bottom->height;
            drawData.yOffset       = seg->yOffset
                                   + ceilingHeightBack
                                   - floorHeightFront;
            drawData.textureID     = seg->top->textureID;

            seg->seen |= drawWall(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);

            drawData.zBottom        = floorHeightFront;

            drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
        }
        else if(ceilingHeightFront <= ceilingHeightBack) //ONLY BOTTOM (and planes)
        {
            drawData.zBottom       = floorHeightFront;
            drawData.zTop          = floorHeightBack;
            drawData.zPos          = pov->zPos + pov->camZ;
            drawData.textureWidth  = seg->bottom->width;
            drawData.textureHeight = seg->bottom->height;
            drawData.yOffset       = seg->yOffset;
            drawData.textureID     = seg->bottom->textureID;

            seg->seen |= drawWall(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);

            drawData.zTop          = ceilingHeightFront;

            drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
        }
        else //BOTH TOP AND BOTTOM (plus planes)
        {
            drawData.zPos          = pov->zPos + pov->camZ;
            drawData.textureWidth  = seg->bottom->width;
            drawData.textureHeight = seg->bottom->height;
            drawData.yOffset       = seg->yOffset;
            drawData.textureID     = seg->bottom->textureID;

            drawData.zBottom       = floorHeightFront;
            drawData.zTop          = floorHeightBack;
            seg->seen |= drawWall(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);

            drawData.zBottom       = ceilingHeightBack;
            drawData.zTop          = ceilingHeightFront;
            drawData.yOffset       = seg->yOffset
                                   + ceilingHeightBack
                                   - floorHeightFront;
            drawData.textureID     = seg->top->textureID;
            seg->seen |= drawWall(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);

            drawData.zBottom       = floorHeightFront;

            drawPlanes(&drawData, SCREENXPOS, SCREENYPOS, SCREENWIDTH, SCREENHEIGHT);
        }
    }

    return;
}


void drawMapSegment(Segment* seg, Entity* pov, float scale)
{
    float scaleFactor;
    float povX;
    float povY;
    float segStartX;
    float segStartY;
    float segDeltaX;
    float segDeltaY;
    float segEndX;
    float segEndY;
    float drawX;
    float drawY;
    float drawDX;
    float drawDY;
    float endX;
    float endY;
    float topInt;
    float bottomInt;
    float leftInt;
    float rightInt;
    float length;
    float tmp;


    //Map scalling
    scaleFactor = scale;

    povX = pov->xPos;
    povY = pov->yPos;

    segStartX =  seg->xPos - pov->xPos;
    segStartY = -seg->yPos + pov->yPos;

    segDeltaX =  seg->dx;
    segDeltaY = -seg->dy;

    segEndX   = segStartX + segDeltaX;
    segEndY   = segStartY + segDeltaY;

    //Rotate points
    drawX = pov->dirSin * segStartX + pov->dirCos * segStartY;
    drawY = pov->dirSin * segStartY - pov->dirCos * segStartX;

    drawDX = pov->dirSin * segDeltaX + pov->dirCos * segDeltaY;
    drawDY = pov->dirSin * segDeltaY - pov->dirCos * segDeltaX;

    //Scale points
    drawX *= scaleFactor;
    drawY *= scaleFactor;
    drawDX *= scaleFactor;
    drawDY *= scaleFactor;

    //Center points
    drawX += MAPWIDTH / 2;
    drawY += MAPHEIGHT /2;

    endX = drawX + drawDX;
    endY = drawY + drawDY;

    //Offscreen clipping
    if(drawX < 0 && endX < 0) //left screen clip
    {
        return;
    }
    else if(drawX > MAPWIDTH && endX > MAPWIDTH) //right screen clip
    {
        return;
    }
    else if(drawY < 0 && endY < 0) //Above screen clip
    {
        return;
    }
    else if(drawY > MAPHEIGHT && endY > MAPHEIGHT) //below screen clip
    {
        return;
    }

    if(drawDX != 0.0)
    {
        if(drawX < 0 || endX < 0)
        {
            leftInt = -(drawDY / drawDX) * drawX + drawY;
        }

        if(drawX > MAPWIDTH || endX > MAPWIDTH)
        {
            rightInt = -(drawDY / drawDX) * (drawX - MAPWIDTH) + drawY;
        }
    }

    if(drawDY != 0.0)
    {
        if(drawY < 0 || endY < 0)
        {
            topInt = -(drawDX / drawDY) * drawY + drawX;
        }

        if(drawY > MAPHEIGHT || endY > MAPHEIGHT)
        {
            bottomInt = -(drawDX / drawDY) * (drawY - MAPHEIGHT) + drawX;
        }
    }

    if(drawX < 0)
    {
        drawX = 0;
        drawY = leftInt;
    }

    if(drawX > MAPWIDTH)
    {
        drawX = MAPWIDTH;
        drawY = rightInt;
    }

    if(drawY < 0)
    {
        drawY = 0;
        drawX = topInt;
    }

    if(drawY > MAPHEIGHT)
    {
        drawY = MAPHEIGHT;
        drawX = bottomInt;
    }

    if(endX < 0)
    {
        endX = 0;
        endY = leftInt;
    }

    if(endX > MAPWIDTH)
    {
        endX = MAPWIDTH;
        endY = rightInt;
    }

    if(endY < 0)
    {
        endY = 0;
        endX = topInt;
    }

    if(endY > MAPHEIGHT)
    {
        endY = MAPHEIGHT;
        endX = bottomInt;
    }



    //Create line
    tmp     = endX - drawX;
    length  = tmp  * tmp;
    tmp     = endY - drawY;
    length += tmp  * tmp;
    length  = sqrt(length);
    set_drawing_scale(length, 1.0);
    set_drawing_angle(atan2(segDeltaY, segDeltaX) + pov->direction - PIo2);

    //Color line
    set_multiply_color(color_white);

    if(seg->isPortal)
      set_multiply_color(color_orange);

    if(seg->isSkyBox)
      set_multiply_color(color_blue);

    draw_region_rotozoomed_at(drawX + MAPXPOS, drawY + MAPYPOS);
}


void drawBspLeafMap(BspLeaf* leaf, Entity* pov, float scale)
{
    int      segSize = sizeof(Segment*);
    Segment** segList = leaf->segList;
    while(*segList != NULL)
    {
        if((*segList) -> seen)
        {
            drawMapSegment(*segList, pov, scale);
        }
        segList += segSize;
    }

    return;
}


void mapBspRender(BspBranch* currentNode, Entity* pov, float scale)
{
    if(currentNode->leaf != NULL)
    {
        drawBspLeafMap(currentNode->leaf, pov, scale);
    }

    if(currentNode->leftNode != NULL)
    {
        mapBspRender(currentNode->leftNode, pov, scale);
    }

    if(currentNode->rightNode != NULL)
    {
        mapBspRender(currentNode->rightNode, pov, scale);
    }

    return;
}


void mapRender(BspBranch* map, Entity* pov, float scale)
{
    select_texture(-1);
    select_region(256);

    mapBspRender(map, pov, scale);

    set_multiply_color(color_magenta);

    draw_region_at(MAPXPOS + MAPWIDTH / 2, MAPYPOS + MAPHEIGHT / 2);

    set_multiply_color(color_white);
}


void drawBspLeaf(FrameBuffer* clipping, BspLeaf* leaf, Entity* pov, int ID)
{
    int      segSize = sizeof(Segment*);

    Segment** segList = leaf->segList;
    while(*segList != NULL)
    {
        if(onRightSideSeg(*segList, pov->xPos, pov->yPos))
        {
            drawSegment(clipping, *segList, pov, ID);
        }
        segList += segSize;
    }

    EntityList* listNode = leaf->entities;
    while(listNode != NULL)
    {
        drawBillboard(listNode->item, pov);
        listNode = listNode->next;
    }

    segList = leaf->segList;
    while(*segList != NULL)
    {
        if(!onRightSideSeg(*segList, pov->xPos, pov->yPos))
        {
            drawSegment(clipping, *segList, pov, ID);
        }
        segList += segSize;
    }

    return;
}



int bspRender(FrameBuffer* clipping, BspBranch* currentNode, Entity* pov, int ID)
{
    BspBranch* tempBranch;
    bool  side;
    bool[2] sideVisability;
    float xPos = pov->xPos;
    float yPos = pov->yPos;

    if(currentNode->leaf != NULL)
    {
        drawBspLeaf(clipping, currentNode->leaf, pov, ID);
        ID--;
    }

    side = onRightSideBranch(currentNode, xPos, yPos);
    areBranchesVisable(currentNode, pov, sideVisability);

    if(side == RIGHT)
    {
        if(sideVisability[0] == true)
        {
            tempBranch = currentNode->leftNode;
            if(tempBranch != NULL)
                ID = bspRender(clipping, tempBranch, pov, ID);
        }

        if(sideVisability[1] == true)
        {
            tempBranch = currentNode->rightNode;
            if(currentNode->leftNode != NULL)
                ID = bspRender(clipping, tempBranch,  pov, ID);
        }
    }
    else
    {
        if(sideVisability[1] == true)
        {
            tempBranch = currentNode->rightNode;
            if(currentNode->leftNode != NULL)
                ID = bspRender(clipping, tempBranch,  pov, ID);
        }

        if(sideVisability[0] == true)
        {
            tempBranch = currentNode->leftNode;
            if(tempBranch != NULL)
                ID = bspRender(clipping, tempBranch, pov, ID);
        }
    }

    return ID;
}

void clipBspLeaf(FrameBuffer* clipping, BspLeaf* leaf, Entity* pov, int ID)
{
    int      segSize = sizeof(Segment*);
    Segment** segList = leaf->segList;
    while(*segList != NULL)
    {
        if(!(*segList)->isPortal || (*segList)->isSkyBox)
            clipSegment(clipping, *segList, pov, ID);

        segList += segSize;
    }

    return;
}

int bspPreRender(FrameBuffer* clipping, BspBranch* currentNode, Entity* pov, int ID)
{
    BspBranch* tempBranch;
    bool  side;
    bool[2] sideVisability;
    float xPos = pov->xPos;
    float yPos = pov->yPos;

    if(currentNode->leaf != NULL)
    {
        clipBspLeaf(clipping, currentNode->leaf, pov, ID);
        ID++;
    }

    side = onRightSideBranch(currentNode, xPos, yPos);
    areBranchesVisable(currentNode, pov, sideVisability);

    if(side == RIGHT)
    {
        if(sideVisability[1] == true)
        {
            tempBranch = currentNode->rightNode;
            if(tempBranch != NULL)
                ID = bspPreRender(clipping, tempBranch, pov, ID);
        }

        if(sideVisability[0] == true)
        {
            tempBranch = currentNode->leftNode;
            if(currentNode->leftNode != NULL)
                ID = bspPreRender(clipping, tempBranch,  pov, ID);
        }
    }
    else
    {
        if(sideVisability[0] == true)
        {
            tempBranch = currentNode->leftNode;
            if(currentNode->leftNode != NULL)
                ID = bspPreRender(clipping, tempBranch,  pov, ID);
        }

        if(sideVisability[1] == true)
        {
            tempBranch = currentNode->rightNode;
            if(tempBranch != NULL)
                ID = bspPreRender(clipping, tempBranch, pov, ID);
        }
    }

    return ID;
}

void drawSkyBox(SkyBox* sky, Entity* pov)
{
    Texture* skyTexture = sky->texture;
    select_texture(skyTexture->textureID);

    float xScale = (float)SCREENWIDTH / ((float)skyTexture->width / 4.0);
    float yScale = (float)SCREENHEIGHT / (float)skyTexture->height;


    float startAngle = PIo4 + pov->direction - sky->rotation;
    float endAngle   = startAngle - PIo2;

    float startTexture = (1.0 - fmod(fmod((startAngle / TAU), 1.0) + 1.0, 1.0))
                       * (float)skyTexture->width;

    float endTexture = (1.0 - fmod(fmod((endAngle / TAU), 1.0) + 1.0, 1.0))
                     * (float)skyTexture->width;

    if(startTexture < endTexture) //No seam
    {
        //left edge
        define_region(
            floor(startTexture),
            0,
            floor(startTexture),
            skyTexture->height,
            floor(startTexture),
            0
        );

        set_drawing_scale(SCREENWIDTH / 2, yScale);
        draw_region_zoomed_at(SCREENXPOS, SCREENYPOS);

        //right edge
        define_region(
            floor(endTexture),
            0,
            floor(endTexture),
            skyTexture->height,
            floor(endTexture),
            0
        );

        draw_region_zoomed_at(SCREENWIDTH / 2 + SCREENXPOS, SCREENYPOS);

        //main region
        select_region(0);
        define_region(
            ceil(startTexture),
            0,
            floor(endTexture) - 1,
            skyTexture->height,
            ceil(startTexture),
            0
        );


        set_drawing_scale(xScale, yScale);
        draw_region_zoomed_at((ceil(startTexture) - startTexture) * xScale + SCREENXPOS, SCREENYPOS);

    }
    else //Seam
    {
        //left edge
        define_region(
            floor(startTexture),
            0,
            floor(startTexture),
            skyTexture->height,
            floor(startTexture),
            0
        );

        set_drawing_scale(SCREENWIDTH / 2, yScale);
        draw_region_zoomed_at(SCREENXPOS, SCREENYPOS);

        //right edge
        define_region(
            floor(endTexture),
            0,
            floor(endTexture),
            skyTexture->height,
            floor(endTexture),
            0
        );

        draw_region_zoomed_at(SCREENWIDTH / 2 + SCREENXPOS, SCREENYPOS);

        //left region
        select_region(0);
        define_region(
            ceil(startTexture),
            0,
            skyTexture->width,
            skyTexture->height,
            ceil(startTexture),
            0
        );

        set_drawing_scale(xScale, yScale);
        draw_region_zoomed_at((ceil(startTexture) - startTexture) * xScale + SCREENXPOS, SCREENYPOS);

        //right region
        define_region(
            0,
            0,
            floor(endTexture) - 1,
            skyTexture->height,
            ceil(startTexture) - skyTexture->width,
            0
        );

        draw_region_zoomed_at((ceil(startTexture) - startTexture) * xScale + SCREENXPOS, SCREENYPOS);
    }

    return;
}


#endif
