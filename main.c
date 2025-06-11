#include "video.h"
#include "math.h"
#include "string.h"
#include "time.h"
#include "misc.h"
#include "input.h"

#define SCREENWIDTH   640
#define SCREENCENTERX 320.0
#define SCREENHEIGHT  320
#define SCREENCENTERY 160.0

#define CURRENTTOP    R10

struct Player
{
    float xPos;
    float yPos;
    float zPos;
    float direction;
    float dirSin;
    float dirCos;
};

struct Texture
{
    int textureID;
    int xCord;
    int yCord;
    int width;
    int height;

};

struct Sector
{
    float floorHeight;
    float ceilingHeight;
    float roomHeight;
    Texture* floor;
    Texture* ceiling;
};

struct Segment
{
    float xPos;
    float yPos;
    float dx;
    float dy;
    float length;
    Sector* sectorLeft;
    Sector* sectorRight;
    Texture* bottom;
    Texture* middle;
    Texture* top;
    float xOffset;
    float yOffset;
};

struct FrameBuffer
{
    int[SCREENWIDTH]   fast;
    int[SCREENWIDTH*2] full;
};

union data
{
    int   in;
    float fl;
};

bool onLeftSide(Segment* seg, float x, float y)
{
    float dx;
    float dy;

    dx = x - seg->xPos;
    dy = y - seg->yPos;

    float left  = dx * seg->dy;
    float right = dy * seg->dx;
    if(right < left)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


void drawSegment(FrameBuffer* clipping, Segment* seg, Player* pov)
{
    int[20] text;

    float povX = pov->xPos;
    float povY = pov->yPos;

    if(!onLeftSide(seg, povX, povY)) //Backface culling
    {
        return;
    }

    data segStartX;
    data segEndX;

    data segStartY;
    data segEndY;

    float segDeltaX;
    float segDeltaY;

    segStartX.fl = pov->dirCos * (seg->xPos - povX)
                 - pov->dirSin * (seg->yPos - povY);

    segDeltaX    = pov->dirCos * seg->dx
                 - pov->dirSin * seg->dy;

    segEndX.fl   = segStartX.fl + segDeltaX;

    if((segStartX.in & segEndX.in) < 0) //Bing
    {
        return;
    }


    segStartY.fl = pov->dirSin * (seg->xPos - povX)
                 + pov->dirCos * (seg->yPos - povY);

    segDeltaY    = pov->dirSin * seg->dx
                 + pov->dirCos * seg->dy;

    segEndY.fl   = segStartY.fl + segDeltaY;

    //return;

    data temp;
    temp.in = segStartX.in & 0x7FFFFFFF;
    if(segStartY.fl > temp.fl) //Start point culling
    {
        return;
    }

    temp.in = segEndX.in | 0x80000000;
    if(segEndY.fl < temp.fl) //End point culling
    {
        return;
    }

    float drawStartX;
    float drawStartY;
    float drawEndX;
    float drawEndY;

    float textureXstart;
    float textureXend;

    if(segStartY.fl < segStartX.fl * -1.0) //Start value adjustment needed
    {
        drawStartY = ((segStartY.fl * segDeltaX) - (segStartX.fl * segDeltaY))
                   / (segDeltaX + segDeltaY);
        drawStartX = -1.0 * drawStartY;

        textureXstart = seg->xOffset + sqrt(
                        (drawStartX - segStartX.fl)*(drawStartX - segStartX.fl)
                      + (drawStartX + segStartY.fl)*(drawStartX + segStartY.fl));
    }
    else
    {
        drawStartX = segStartX.fl;
        drawStartY = segStartY.fl;
        textureXstart = seg->xOffset;
    }


    if(segEndY.fl > segEndX.fl) //End Value adjustment needed
    {
        drawEndY   = (segStartX.fl * segDeltaY - segStartY.fl * segDeltaX)
                   / (segDeltaY - segDeltaX);
        drawEndX   = drawEndY;
        textureXend   = (seg->length - sqrt(
                        (segEndX.fl - drawEndX)*(segEndX.fl - drawEndX)
                      + (segEndY.fl - drawEndX)*(segEndY.fl - drawEndX)))
                      + seg->xOffset;
    }
    else
    {
        drawEndX = segEndX.fl;
        drawEndY = segEndY.fl;
        textureXend = seg->xOffset + seg->length;
    }

    float onScreenWidth = SCREENWIDTH * (drawEndY/drawEndX - drawStartY/drawStartX) / 2;

    temp.fl = pov->zPos;

    float topStart    =  SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorLeft->ceilingHeight - temp.fl) / drawStartX);

    float topEnd      = SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorLeft->ceilingHeight - temp.fl) / drawEndX);

    float bottomStart =  SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorLeft->floorHeight   - temp.fl) / drawStartX);

    float bottomEnd   = SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorLeft->floorHeight   - temp.fl) / drawEndX);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float textureWidth  = seg->middle->width;
    float textureHeight = seg->middle->height;

    float currentTextureScale = (bottomStart - topStart) / seg->sectorLeft->roomHeight;
    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
                              / (seg->sectorLeft->roomHeight * onScreenWidth);

    float textureYoffset = seg->yOffset;

    float progress      = 0.0;
    float remaining     = 1.0;
    float progresStep   = 1 / onScreenWidth;
    int   column        = SCREENCENTERX + SCREENWIDTH * (drawStartY/drawStartX) / 2;

    float texOverXstart = textureXstart / drawStartX;
    float texOverXend   = textureXend   / drawEndX;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / drawStartX;
    float inverseXend   = 1.0 / drawEndX;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    float roomHeight        = seg->sectorLeft->roomHeight;
    float textureTrueTop    = textureHeight - textureYoffset - roomHeight;
    float textureTrueBottom = textureHeight - textureYoffset;
    int*  clippingPointer   = &(clipping->full[column*2]);
    int   screenClipTop;
    int   screenClipBottom;
    int   screenTop;
    int   screenBottom;
    int   screenFullTop;
    int   screenFullBottom;
    float textureTop;
    float textureBottom;
    int   textureFullTop;
    int   textureFullBottom;

    select_texture(seg->middle->textureID);
    set_drawing_scale(1.0, 1.0);
    select_region(0);

    asm
    {
        //Initialize registers
        "mov  R4,  {texOverXstart}"
        "mov  R5,  {inverseXstart}"
        "mov  R6,  {remaining}"
        "mov  R7,  {progress}"
        "mov  R8,  {currentTextureScale}"
        "mov  R9,  {currentBottom}"
        "mov  R10, {currentTop}"
        "mov  R13, {clippingPointer}"

        //Loop start
        "_wall_while_loop_start:"

        //Check loop condition
        "mov  R0,  R7"
        "flt  R0,  1.0"
        "jf   R0,  _wall_while_loop_end"

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

        "mov  R0, {column}"
        "out  GPU_DrawingPointX,  R0"

        //Get clipping
        "mov  R12, [R13]"
        "iadd R13, 1"
        "mov  R11, [R13]"
        "iadd R13, 1"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "mov  R1,  R11"
        "cif  R1"
        "fgt  R0,  R1"
        "jf   R0,  _bottom_not_clipped"

        //Bottom is clipped

        // screenBottom
        "mov  {screenBottom}, R11"
        "mov  R1,  {screenBottom}"

        // textureBottom
        "mov  R0,  {textureTrueBottom}"
        "mov  R2,  R9"
        "cif  R1"
        "fsub R2,  R1"
        "fdiv R2,  R8"
        "fsub R0,  R2"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0,  R2"
        "fmul R0,  R8"
        "fsub R1,  R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "jmp  _bottom_clipped_end"



        //Bottom is not clipped
        "_bottom_not_clipped:"

        // screenBottom
        "mov  R1,  R9"
        "cfi  R1"
        "mov  {screenBottom}, R1"
        "mov  R1,  R9"

        // textureBottom
        "mov  R0, {textureTrueBottom}"

        // textureFullBottom
        "mov  R2, R0"
        "flr  R2"

        // screenFullBottom
        "fsub R0, R2"
        "fmul R0, R8"
        "fsub R1, R0"
        "cfi  R1"
        "mov  {screenFullBottom}, R1"

        "cfi  R2"
        "mov  {textureFullBottom}, R2"

        "_bottom_clipped_end:"




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
        "cif  R1"
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
        "igt  R2, R3"
        "jf   R2, _fullTextureSkip"

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
        "ine  R0,  R1"
        "jf   R0,  _topTextureSkip"

        //Texture Top Drawing
        "mov  R0,  {screenFullTop}"
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
        "ine  R0,  R1"
        "jf   R0,  _bottomTextureSkip"

        //Texture Bottom Drawing
        "mov  R0,  {screenFullBottom}"
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



        // While iterators
        "mov  R1,  {topStep}"
        "fadd R10, R1"

        "mov  R1,  {bottomStep}"
        "fadd R9,  R1"

        "mov  R1,  {textureScaleStep}"
        "fadd R8,  R1"

        "mov  R1,  {progresStep}"
        "fadd R7,  R1"
        "fsub R6,  R1"

        "mov  R1,  {inverseXstep}"
        "fadd R5,  R1"

        "mov  R1,  {texOverXstep}"
        "fadd R4,  R1"

        "mov  R0,  {column}"
        "iadd R0,  1"
        "mov  {column}, R0"

        "jmp  _wall_while_loop_start"

        "_wall_while_loop_end:"
    }


    return;
}


void main(void)
{
    Player      user;
    Texture     emptyTexture;
    Sector      Room0;
    Sector      Room1;
    Segment     wall1;
    Segment     wall2;
    FrameBuffer drawClip;
    FrameBuffer cleanBuffer;

    user.xPos = 0.00;
    user.yPos = 1.00;
    user.zPos = 2.00;
    user.direction = 0.0;
    user.dirSin = sin(user.direction);
    user.dirCos = cos(user.direction);

    emptyTexture.textureID = 1;
    emptyTexture.xCord  = 0;
    emptyTexture.yCord  = 0;
    emptyTexture.width  = 32;
    emptyTexture.height = 32;

    Room0.floorHeight   =  0.0;
    Room0.ceilingHeight =  20.0;
    Room0.roomHeight    = Room0.ceilingHeight - Room0.floorHeight;
    Room0.floor         = &emptyTexture;
    Room0.ceiling       = &emptyTexture;

    Room1 = Room0;

    wall1.xPos        =  1.0;
    wall1.yPos        = -13.0;
    wall1.dx          =  5.0;
    wall1.dy          =  14.0;
    wall1.length      = sqrt(wall1.dx * wall1.dx + wall1.dy * wall1.dy);
    wall1.sectorLeft  = &Room0;
    wall1.sectorRight = &Room1;
    wall1.bottom      = &emptyTexture;
    wall1.middle      = &emptyTexture;
    wall1.top         = &emptyTexture;
    wall1.yOffset     =  0.5;
    wall1.xOffset     =  0.0;

    wall2.xPos        =  6.0;
    wall2.yPos        =  1.0;
    wall2.dx          = -8.0;
    wall2.dy          =  6.0;
    wall2.length      = sqrt(wall2.dx * wall2.dx + wall2.dy * wall2.dy);
    wall2.sectorLeft  = &Room0;
    wall2.sectorRight = &Room1;
    wall2.bottom      = &emptyTexture;
    wall2.middle      = &emptyTexture;
    wall2.top         = &emptyTexture;
    wall2.yOffset     =  0.0;
    wall2.xOffset     =  0.0;

    for(int i = 0; i < SCREENWIDTH; i++)
    {
        drawClip.fast[i] = false;
    }
    for(int i = 0; i < SCREENWIDTH*2; i += 2)
    {
        drawClip.full[i]   = 0;
        drawClip.full[i+1] = SCREENHEIGHT - 1;
    }
    cleanBuffer = drawClip;

    float speedX;
    float speedY;

    while(true)
    {
        clear_screen(color_gray);
        drawSegment(&drawClip, &wall1, &user);
        drawSegment(&drawClip, &wall2, &user);

        if(gamepad_button_b() > 0)
            user.direction += 0.001 * gamepad_button_b();
        if(gamepad_button_a() > 0)
            user.direction -= 0.001 * gamepad_button_a();

        user.dirCos = cos(user.direction);
        user.dirSin = sin(user.direction);

        speedX = 0.0;
        speedY = 0.0;

        if(gamepad_left() > 0)
            speedY -= 0.01 * gamepad_left();
        if(gamepad_right() > 0)
            speedY += 0.01 * gamepad_right();
        if(gamepad_up() > 0)
            speedX += 0.01 * gamepad_up();
        if(gamepad_down() > 0)
            speedX -= 0.01 * gamepad_down();

        user.xPos += user.dirCos * speedX + user.dirSin * speedY;
        user.yPos += user.dirCos * speedY - user.dirSin * speedX;
        end_frame();
    }

}
