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
    int width;
    int height;

};

struct Sector
{
    float floorHeight;
    float ceilingHeight;
    float roomHeight;
    int   floorColor;
    int   ceilingColor;
};

struct Segment
{
    float    xPos;
    float    yPos;
    float    dx;
    float    dy;
    float    length;
    bool     isPortal;
    Sector*  sectorRight;
    Sector*  sectorLeft;
    Texture* bottom;
    Texture* middle;
    Texture* top;
    float    xOffset;
    float    yOffset;
};

struct BspLeaf
{
    Segment** segList;
};

struct BspBranch
{
    float      HyperX;
    float      HyperY;
    float      HyperDy;
    float      HyperDx;
    bool       BranchSide;
    BspBranch* parentNode;
    BspBranch* rightNode;
    BspBranch* leftNode;
    BspLeaf*   leaf;
};

struct FrameBuffer
{
    int[SCREENWIDTH]   fast;
    int[SCREENWIDTH*2] full; //(Bottom, Top)//
};

union data
{
    int   in;
    float fl;
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

bool onRightSideSeg(Segment* seg, float x, float y)
{
    float dx = x - seg->xPos;
    float dy = y - seg->yPos;

    float left  = dx * seg->dy;
    float right = dy * seg->dx;
    if(right < left)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool onRightSideBranch(BspBranch* branch, float x, float y)
{
    float dx;
    float dy;

    dx = x - branch->HyperX;
    dy = y - branch->HyperY;

    float left  = dx * branch->HyperDy;
    float right = dy * branch->HyperDx;
    if(right < left)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



void drawPortalClipBottom(WallDrawData* data)
{
    float        xStart        = data->xStart;
    float        xEnd          = data->xEnd;
    float        yStart        = data->yStart;
    float        yEnd          = data->yEnd;
    float        zTop          = data->zTop;
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
    set_multiply_color(floorColor);

    asm
    {
        // Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R9,  {currentTop}"
        "mov  R10, {topStep}"
        "mov  R13, {fullClippingPointer}"

        // Loop start
        "_BCwall_while_loop_start:"

        // Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _BCwall_while_loop_iterators"

        // Get bottom clipping
        "mov  R12, [R13]"

        // Check bottom clipping
        "mov  R0,  R9"
        "cfi  R0"
        "ige  R0,  R12"
        "jt   R0,  _BCwall_while_loop_iterators"

        // Get top clipping
        "iadd R13, 1"
        "mov  R11, [R13]"

        // Get top of flat
        "mov  R0,  R9"
        "cfi  R0"
        "imax R0,  R11"

        // Draw floor line
        "mov  R1,  R12"
        "isub R1,  R0"
        "cif  R1"
        "out  GPU_DrawingScaleY,  R1"
        "out  GPU_DrawingPointX,  R6"
        "out  GPU_DrawingPointY,  R0"

        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        // Update clipping
        "isub R13, 1"
        "mov  [R13], R0"

        "_BCwall_while_loop_iterators:"

        // While iterators
        "iadd R6,  1"
        "iadd R7,  1"
        "isub R8,  1"
        "fadd R9,  R10"
        "iadd R13, 2"

        // Loop condition
        "jt   R8, _BCwall_while_loop_start"

        "_BCwall_while_loop_end:"
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
    float        zBottom       = data->zBottom;
    float        zPos          = data->zPos;
    FrameBuffer* clipping      = data->clipping;
    int          ceilingColor  = data->ceilingColor;


    int   columnStart = min(floor((yStart/xStart - 1.0) * -320.0), 640);
    int   columnEnd   = min(floor((yEnd/xEnd - 1.0) * -320.0), 640);

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
    set_multiply_color(ceilingColor);

    asm
    {
        // Initialize registers
        "mov  R6,  {columnStart}"
        "mov  R7,  {fastClippingPointer}"
        "mov  R8,  {onScreenWidth}"
        "cfi  R8"
        "iadd R8,  1"
        "mov  R9,  {currentBottom}"
        "mov  R10, {bottomStep}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_TCwall_while_loop_start:"

        // Check fast clipping
        "mov  R0,  [R7]"
        "jt   R0,  _TCwall_while_loop_iterators"

        //Get bottom clipping
        "mov  R12,  [R13]"

        //Get top clipping
        "iadd R13, 1"
        "mov  R11, [R13]"

        // Check top clipping
        "mov  R0,  R9"
        "cfi  R0"
        "ile  R0,  R11"
        "jt   R0,  _TCwall_while_loop_iterators_skip"

        // Get bottom of flat
        "mov  R0,  R9"
        "cfi  R0"
        "imin R0,  R12"

        // Draw floor line
        "mov  R1,  R0"
        "isub R1,  R11"
        "cif  R1"
        "out  GPU_DrawingScaleY,  R1"
        "out  GPU_DrawingPointX,  R6"
        "out  GPU_DrawingPointY,  R11"

        "out  GPU_Command, GPUCommand_DrawRegionZoomed"

        // Update clipping
        "mov  [R13], R0"
        "isub R13, 1"


        "_TCwall_while_loop_iterators:"
        "iadd R13, 1"

        "_TCwall_while_loop_iterators_skip:"

        // While iterators
        "iadd R6,  1"
        "iadd R7,  1"
        "isub R8,  1"
        "fadd R9,  R10"
        "iadd R13, 1"

        // Loop condition
        "jt   R8, _TCwall_while_loop_start"

        "_TCwall_while_loop_end:"
    }

    set_multiply_color(color_white);

    return;
}


void drawPortal(
    WallDrawData* data,
    float windowBottomZ,
    float windowTopZ,
    int textureTop
)
{
    float        xStart           = data->xStart;
    float        xEnd             = data->xEnd;
    float        yStart           = data->yStart;
    float        yEnd             = data->yEnd;
    float        textureStart     = data->textureStart;
    float        textureEnd       = data->textureEnd;
    float        zBottom          = data->zBottom;
    float        zTop             = data->zTop;
    float        zPos             = data->zPos;
    float        textureWidth     = data->textureWidth;
    float        textureHeight    = data->textureHeight;
    float        yOffset          = data->yOffset;
    FrameBuffer* clipping         = data->clipping;
    float        roomHeight       = zTop - zBottom;
    float        topWallHeight    = zTop - windowTopZ;
    float        bottomWallHeight = windowBottomZ - zBottom;
    int          floorColor       = data->floorColor;
    int          ceilingColor     = data->ceilingColor;
    int          textureBottom    = data->textureID;

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

    float textureTrueTop          = textureHeight - yOffset - roomHeight;
    float textureTrueWindowTop    = textureTrueTop + topWallHeight;
    float textureTrueBottom       = textureHeight - yOffset;
    float textureTrueWindowBottom = textureTrueBottom - bottomWallHeight;
    int*  fullClippingPointer = &(clipping->full[columnStart*2]);
    int*  fastClippingPointer = &(clipping->fast[0]);
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


        // Go to next full clipping
        "_bottom_offsecreen:"
        "_top_offscreen:"
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

    set_multiply_color(color_white);

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

            segDeltaX    = pov->dirCos * seg->dx
                        + pov->dirSin * seg->dy;

            segStartX.fl   = segEndX.fl + segDeltaX;

            if((segStartX.in & segEndX.in) < 0) //Behind culling
            {
                return;
            }


            segEndY.fl = pov->dirCos * (seg->yPos - povY)
                        - pov->dirSin * (seg->xPos - povX);

            segDeltaY    = pov->dirCos * seg->dy
                        - pov->dirSin * seg->dx;

            segStartY.fl   = segEndY.fl + segDeltaY;

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

    if(backFace == true)
    {
        drawData.xStart         = drawStartX;
        drawData.xEnd           = drawEndX;
        drawData.yStart         = drawStartY;
        drawData.yEnd           = drawEndY;
        drawData.textureStart   = textureXstart;
        drawData.textureEnd     = textureXend;
        drawData.zBottom        = seg->sectorRight->floorHeight;
        drawData.zTop           = seg->sectorLeft->floorHeight;
        drawData.zPos           = pov->zPos;
        drawData.textureWidth   = seg->bottom->width;
        drawData.textureHeight  = seg->bottom->height;
        drawData.yOffset        = seg->yOffset;
        drawData.clipping       = clipping;
        drawData.floorColor     = seg->sectorLeft->floorColor;

        drawPortalClipBottom(&drawData);

        drawData.zBottom        = seg->sectorLeft->ceilingHeight;
        drawData.zTop           = seg->sectorRight->ceilingHeight;
        drawData.zPos           = pov->zPos;
        drawData.ceilingColor   = seg->sectorLeft->ceilingColor;

        drawPortalClipTop(&drawData);
    }
    else if(seg->isPortal == false)
    {
        drawData.xStart         = drawStartX;
        drawData.xEnd           = drawEndX;
        drawData.yStart         = drawStartY;
        drawData.yEnd           = drawEndY;
        drawData.textureStart   = textureXstart;
        drawData.textureEnd     = textureXend;
        drawData.zBottom        = seg->sectorRight->floorHeight;
        drawData.zTop           = seg->sectorRight->ceilingHeight;
        drawData.zPos           = pov->zPos;
        drawData.textureWidth   = seg->middle->width;
        drawData.textureHeight  = seg->middle->height;
        drawData.yOffset        = seg->yOffset;
        drawData.clipping       = clipping;
        drawData.floorColor     = seg->sectorRight->floorColor;
        drawData.ceilingColor   = seg->sectorRight->ceilingColor;
        drawData.textureID      = seg->middle->textureID;

        drawWall(&drawData);
    }
    else
    {
        drawData.xStart         = drawStartX;
        drawData.xEnd           = drawEndX;
        drawData.yStart         = drawStartY;
        drawData.yEnd           = drawEndY;
        drawData.textureStart   = textureXstart;
        drawData.textureEnd     = textureXend;
        drawData.zBottom        = seg->sectorRight->floorHeight;
        drawData.zTop           = seg->sectorRight->ceilingHeight;
        drawData.zPos           = pov->zPos;
        drawData.textureWidth   = seg->bottom->width;
        drawData.textureHeight  = seg->bottom->height;
        drawData.yOffset        = seg->yOffset;
        drawData.clipping       = clipping;
        drawData.textureID      = seg->bottom->textureID;
        drawData.floorColor     = seg->sectorRight->floorColor;
        drawData.ceilingColor   = seg->sectorRight->ceilingColor;

        drawPortal(
            &drawData,
            seg->sectorLeft->floorHeight,
            seg->sectorLeft->ceilingHeight,
            seg->top->textureID
        );
    }

    inputWait();
    return;
}


void drawBspLeaf(FrameBuffer* clipping, BspLeaf* leaf, Player* pov)
{
    int      segSize = sizeof(BspLeaf);
    Segment** segList = leaf->segList;
    while(*segList != NULL)
    {
        drawSegment(clipping, *segList, pov);
        segList += segSize;
    }

    return;
}


BspBranch* locateBsp(BspBranch* currentNode, float xPos, float yPos)
{
    while(currentNode->leftNode != NULL)
    {
        if(onRightSideBranch(currentNode, xPos, yPos))
        {
            currentNode = currentNode->leftNode;
        }
        else
        {
            currentNode = currentNode->rightNode;
        }
    }

    return currentNode;
}

void bspRender(
    FrameBuffer* clipping,
    BspBranch* currentNode,
    Player* pov
)
{
    BspBranch* tempBranch;
    bool  side;
    float xPos = pov->xPos;
    float yPos = pov->yPos;

    if(currentNode->leaf != NULL)
    {
        drawBspLeaf(clipping, currentNode->leaf, pov);
    }

    side = onRightSideBranch(currentNode, xPos, yPos);

    if(side == RIGHT)
    {
        tempBranch = currentNode->rightNode;
        if(tempBranch != NULL)
            bspRender(clipping, tempBranch, pov);

        tempBranch = currentNode->leftNode;
        if(currentNode->leftNode != NULL)
            bspRender(clipping, tempBranch,  pov);
    }
    else
    {
        tempBranch = currentNode->leftNode;
        if(currentNode->leftNode != NULL)
            bspRender(clipping, tempBranch,  pov);

        tempBranch = currentNode->rightNode;
        if(tempBranch != NULL)
            bspRender(clipping, tempBranch, pov);
    }

    return;
}

void main(void)
{
    int         TIME;
    Player      user;
    Texture     wallTexture;
    Texture     testTexture;
    FrameBuffer drawClip;
    FrameBuffer cleanBuffer;
    Sector      Room0;
    Sector      Room1;
    Segment     wall0;
    Segment     wall1;
    Segment     wall2;
    Segment     wall3;
    Segment     wall4;
    Segment     wall5;
    Segment     wall6;
    Segment     wall7;
    Segment     wall8;
    Segment     wall9;
    Segment     wall10;
    BspLeaf     leaf0;
    BspLeaf     leaf1;
    BspLeaf     leaf2;
    BspLeaf     leaf3;
    BspBranch   rootNode;
    BspBranch   node0;
    BspBranch   node1;
    BspBranch   node2;
    BspBranch   node3;
    BspBranch   node4;
    BspBranch   node5;

    user.xPos =    111.00;
    user.yPos =     12.00;
    user.zPos =     16.00;
    user.direction = 0.0;
    user.dirSin = sin(user.direction);
    user.dirCos = cos(user.direction);

    wallTexture.textureID = 1;
    wallTexture.width  = 32;
    wallTexture.height = 32;

    testTexture.textureID = 0;
    testTexture.width  = 32;
    testTexture.height = 32;

    for(int i = 0; i < SCREENWIDTH; i++)
    {
        drawClip.fast[i] = false;
    }
    for(int i = 0; i < SCREENWIDTH*2; i += 2)
    {
        drawClip.full[i]   = SCREENHEIGHT - 1;
        drawClip.full[i+1] = 10;
    }
    cleanBuffer = drawClip;

    Room0.floorHeight   =  0.0;
    Room0.ceilingHeight = 32.0;
    Room0.floorColor    = 0xFF88FF88;
    Room0.ceilingColor  = 0xFFFFFF88;

    Room1.floorHeight   = 10.0;
    Room1.ceilingHeight = 22.0;
    Room1.floorColor    = 0xFF0000FF;
    Room1.ceilingColor  = 0xFFFF0000;

    wall0.xPos        =   48.0;
    wall0.yPos        =   96.0;
    wall0.dx          =   80.0;
    wall0.dy          =    0.0;
    wall0.length      = sqrt(wall0.dx * wall0.dx + wall0.dy * wall0.dy);
    wall0.isPortal    = false;
    wall0.sectorRight = &Room0;
    wall0.sectorLeft  = &Room1;
    wall0.bottom      = &wallTexture;
    wall0.middle      = &wallTexture;
    wall0.top         = &wallTexture;
    wall0.yOffset     =    0.0;
    wall0.xOffset     =   48.0;

    wall1.xPos        =  128.0;
    wall1.yPos        =   96.0;
    wall1.dx          =    0.0;
    wall1.dy          =  -96.0;
    wall1.length      = sqrt(wall1.dx * wall1.dx + wall1.dy * wall1.dy);
    wall1.isPortal    = false;
    wall1.sectorRight = &Room0;
    wall1.sectorLeft  = &Room1;
    wall1.bottom      = &wallTexture;
    wall1.middle      = &wallTexture;
    wall1.top         = &wallTexture;
    wall1.yOffset     =    0.0;
    wall1.xOffset     =    0.0;

    wall2.xPos        =  128.0;
    wall2.yPos        =    0.0;
    wall2.dx          =   -8.0;
    wall2.dy          =    0.0;
    wall2.length      = sqrt(wall2.dx * wall2.dx + wall2.dy * wall2.dy);
    wall2.isPortal    = false;
    wall2.sectorRight = &Room0;
    wall2.sectorLeft  = &Room1;
    wall2.bottom      = &wallTexture;
    wall2.middle      = &wallTexture;
    wall2.top         = &wallTexture;
    wall2.yOffset     =    0.0;
    wall2.xOffset     =    0.0;

    wall3.xPos        =   76.8;
    wall3.yPos        =   57.6;
    wall3.dx          =  -19.2;
    wall3.dy          =   25.6;
    wall3.length      = sqrt(wall3.dx * wall3.dx + wall3.dy * wall3.dy);
    wall3.isPortal    = true;
    wall3.sectorRight = &Room0;
    wall3.sectorLeft  = &Room1;
    wall3.bottom      = &wallTexture;
    wall3.middle      = &wallTexture;
    wall3.top         = &testTexture;
    wall3.yOffset     =    0.0;
    wall3.xOffset     =    0.0;

    wall4.xPos        =  120.0;
    wall4.yPos        =    0.0;
    wall4.dx          = -120.0;
    wall4.dy          =    0.0;
    wall4.length      = sqrt(wall4.dx * wall4.dx + wall4.dy * wall4.dy);
    wall4.isPortal    = false;
    wall4.sectorRight = &Room0;
    wall4.sectorLeft  = &Room1;
    wall4.bottom      = &wallTexture;
    wall4.middle      = &wallTexture;
    wall4.top         = &wallTexture;
    wall4.yOffset     =    0.0;
    wall4.xOffset     =    8.0;

    wall5.xPos        =   51.2;
    wall5.yPos        =   38.4;
    wall5.dx          =   25.6;
    wall5.dy          =   19.2;
    wall5.length      = sqrt(wall5.dx * wall5.dx + wall5.dy * wall5.dy);
    wall5.isPortal    = true;
    wall5.sectorRight = &Room0;
    wall5.sectorLeft  = &Room1;
    wall5.bottom      = &wallTexture;
    wall5.middle      = &wallTexture;
    wall5.top         = &wallTexture;
    wall5.yOffset     =    0.0;
    wall5.xOffset     =    0.0;

    wall6.xPos        =    0.0;
    wall6.yPos        =    0.0;
    wall6.dx          =    0.0;
    wall6.dy          =   96.0;
    wall6.length      = sqrt(wall6.dx * wall6.dx + wall6.dy * wall6.dy);
    wall6.isPortal    = false;
    wall6.sectorRight = &Room0;
    wall6.sectorLeft  = &Room1;
    wall6.bottom      = &wallTexture;
    wall6.middle      = &wallTexture;
    wall6.top         = &wallTexture;
    wall6.yOffset     =    0.0;
    wall6.xOffset     =    0.0;

    wall7.xPos        =    0.0;
    wall7.yPos        =   96.0;
    wall7.dx          =    8.0;
    wall7.dy          =    0.0;
    wall7.length      = sqrt(wall7.dx * wall7.dx + wall7.dy * wall7.dy);
    wall7.isPortal    = false;
    wall7.sectorRight = &Room0;
    wall7.sectorLeft  = &Room1;
    wall7.bottom      = &wallTexture;
    wall7.middle      = &wallTexture;
    wall7.top         = &wallTexture;
    wall7.yOffset     =    0.0;
    wall7.xOffset     =    0.0;

    wall8.xPos        =   32.0;
    wall8.yPos        =   64.0;
    wall8.dx          =   19.2;
    wall8.dy          =  -25.6;
    wall8.length      = sqrt(wall8.dx * wall8.dx + wall8.dy * wall8.dy);
    wall8.isPortal    = true;
    wall8.sectorRight = &Room0;
    wall8.sectorLeft  = &Room1;
    wall8.bottom      = &wallTexture;
    wall8.middle      = &wallTexture;
    wall8.top         = &wallTexture;
    wall8.yOffset     =    0.0;
    wall8.xOffset     =    0.0;

    wall9.xPos        =    8.0;
    wall9.yPos        =   96.0;
    wall9.dx          =   40.0;
    wall9.dy          =    0.0;
    wall9.length      = sqrt(wall9.dx * wall9.dx + wall9.dy * wall9.dy);
    wall9.isPortal    = false;
    wall9.sectorRight = &Room0;
    wall9.sectorLeft  = &Room1;
    wall9.bottom      = &wallTexture;
    wall9.middle      = &wallTexture;
    wall9.top         = &wallTexture;
    wall9.yOffset     =    0.0;
    wall9.xOffset     =    8.0;

    wall10.xPos        =   57.6;
    wall10.yPos        =   83.2;
    wall10.dx          =  -25.6;
    wall10.dy          =  -19.2;
    wall10.length      = sqrt(wall10.dx * wall10.dx + wall10.dy * wall10.dy);
    wall10.isPortal    = true;
    wall10.sectorRight = &Room0;
    wall10.sectorLeft  = &Room1;
    wall10.bottom      = &wallTexture;
    wall10.middle      = &wallTexture;
    wall10.top         = &wallTexture;
    wall10.yOffset     =    0.0;
    wall10.xOffset     =    0.0;

    Segment*[5] leaf0List = {&wall3, &wall0, &wall1, &wall2, NULL};
    leaf0.segList = &(leaf0List[0]);

    Segment*[3] leaf1List = {&wall5, &wall4, NULL};
    leaf1.segList = &(leaf1List[0]);

    Segment*[4] leaf2List = {&wall8, &wall6, &wall7, NULL};
    leaf2.segList = &(leaf2List[0]);

    Segment*[3] leaf3List = {&wall10, &wall9, NULL};
    leaf3.segList = &(leaf3List[0]);

    rootNode.HyperX     =   76.8;
    rootNode.HyperY     =   57.6;
    rootNode.HyperDx    =  -19.2;
    rootNode.HyperDy    =   25.6;
    rootNode.BranchSide = NULL;
    rootNode.rightNode  = &node0;
    rootNode.leftNode   = &node1;
    rootNode.parentNode = NULL;
    rootNode.leaf       = NULL;

    node0.HyperX     = NULL;
    node0.HyperY     = NULL;
    node0.HyperDx    = NULL;
    node0.HyperDy    = NULL;
    node0.BranchSide = RIGHT;
    node0.rightNode  = NULL;
    node0.leftNode   = NULL;
    node0.parentNode = &rootNode;
    node0.leaf       = &leaf0;

    node1.HyperX     =   51.2;
    node1.HyperY     =   38.4;
    node1.HyperDx    =   25.6;
    node1.HyperDy    =   19.2;
    node1.BranchSide = LEFT;
    node1.rightNode  = &node2;
    node1.leftNode   = &node3;
    node1.parentNode = &rootNode;
    node1.leaf       = NULL;

    node2.HyperX     = NULL;
    node2.HyperY     = NULL;
    node2.HyperDx    = NULL;
    node2.HyperDy    = NULL;
    node2.BranchSide = RIGHT;
    node2.rightNode  = NULL;
    node2.leftNode   = NULL;
    node2.parentNode = &rootNode;
    node2.leaf       = &leaf1;

    node3.HyperX     =   32.0;
    node3.HyperY     =   64.0;
    node3.HyperDx    =   19.2;
    node3.HyperDy    =  -25.6;
    node3.BranchSide = LEFT;
    node3.rightNode  = &node4;
    node3.leftNode   = &node5;
    node3.parentNode = &rootNode;
    node3.leaf       = NULL;

    node4.HyperX     = NULL;
    node4.HyperY     = NULL;
    node4.HyperDx    = NULL;
    node4.HyperDy    = NULL;
    node4.BranchSide = RIGHT;
    node4.rightNode  = NULL;
    node4.leftNode   = NULL;
    node4.parentNode = &rootNode;
    node4.leaf       = &leaf2;

    node5.HyperX     = NULL;
    node5.HyperY     = NULL;
    node5.HyperDx    = NULL;
    node5.HyperDy    = NULL;
    node5.BranchSide = LEFT;
    node5.rightNode  = NULL;
    node5.leftNode   = NULL;
    node5.parentNode = &rootNode;
    node5.leaf       = &leaf3;

    float speedX;
    float speedY;

    while(true)
    {
        clear_screen(color_black);

        bspRender(&drawClip, &rootNode, &user);

        if(gamepad_button_b() > 0)
            user.direction += 0.001 * gamepad_button_b();
        if(gamepad_button_a() > 0)
            user.direction -= 0.001 * gamepad_button_a();

        user.dirCos = cos(user.direction);
        user.dirSin = sin(user.direction);

        speedX = 0.0;
        speedY = 0.0;

        if(gamepad_left() > 0)
            speedY += 0.005 * gamepad_left();
        if(gamepad_right() > 0)
            speedY -= 0.005 * gamepad_right();
        if(gamepad_up() > 0)
            speedX += 0.005 * gamepad_up();
        if(gamepad_down() > 0)
            speedX -= 0.005 * gamepad_down();

        user.xPos += user.dirCos * speedX - user.dirSin * speedY;
        user.yPos += user.dirCos * speedY + user.dirSin * speedX;
        user.zPos =  16.0 + 1.5*cos((float)TIME / 300.0);
        Room1.floorHeight = 8.0 - 8.0*sin((float)TIME / 60.0);
        Room1.ceilingHeight = 24.0 + 8.0*sin((float)TIME / 60.0);

        drawClip = cleanBuffer;

        TIME++;
        end_frame();
    }

}
