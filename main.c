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
    Sector* sectorRight;
    Sector* sectorLeft;
    Texture* bottom;
    Texture* middle;
    Texture* top;
    float xOffset;
    float yOffset;
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
    int[SCREENWIDTH*2] full; //(Top, Bottom)//
};

union data
{
    int   in;
    float fl;
};

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

void drawSegment(FrameBuffer* clipping, Segment* seg, Player* pov)
{
    int[20] text;

    float povX = pov->xPos;
    float povY = pov->yPos;

    if(!onRightSideSeg(seg, povX, povY)) //Backface culling
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

    float onScreenWidth = SCREENWIDTH * (drawStartY/drawStartX - drawEndY/drawEndX) / 2;
    if(onScreenWidth == 0.0)
    {
        return;
    }

    temp.fl = pov->zPos;

    float topStart    =  SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorRight->ceilingHeight - temp.fl) / drawStartX);

    float topEnd      = SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorRight->ceilingHeight - temp.fl) / drawEndX);

    float bottomStart =  SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorRight->floorHeight   - temp.fl) / drawStartX);

    float bottomEnd   = SCREENCENTERY
                      - (SCREENHEIGHT * (seg->sectorRight->floorHeight   - temp.fl) / drawEndX);

    float currentTop = topStart;
    float topStep    = (topEnd - topStart) / onScreenWidth;

    float currentBottom = bottomStart;
    float bottomStep    = (bottomEnd - bottomStart) / onScreenWidth;

    float textureWidth  = seg->middle->width;
    float textureHeight = seg->middle->height;

    float currentTextureScale = (bottomStart - topStart) / seg->sectorRight->roomHeight;
    float textureScaleStep    = (bottomEnd - topEnd - bottomStart + topStart)
                              / (seg->sectorRight->roomHeight * onScreenWidth);

    float textureYoffset = seg->yOffset;

    float progress      = 0.0;
    float remaining     = 1.0;
    float progresStep   = 1 / onScreenWidth;
    int   column        = SCREENCENTERX - SCREENWIDTH * (drawStartY/drawStartX) / 2;

    float texOverXstart = textureXstart / drawStartX;
    float texOverXend   = textureXend   / drawEndX;
    float texOverXstep  = (texOverXend - texOverXstart) / onScreenWidth;

    float inverseXstart = 1.0 / drawStartX;
    float inverseXend   = 1.0 / drawEndX;
    float inverseXstep  =  (inverseXend - inverseXstart) / onScreenWidth;

    float roomHeight          = seg->sectorRight->roomHeight;
    float textureTrueTop      = textureHeight - textureYoffset - roomHeight;
    float textureTrueBottom   = textureHeight - textureYoffset;
    int*  fullClippingPointer = &(clipping->full[column*2]);
    int*  fastClippingPointer = &(clipping->fast[0]);
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
        "mov  R11, {column}"
        "mov  R13, {fullClippingPointer}"

        //Loop start
        "_wall_while_loop_start:"

        //Check loop condition
        "mov  R0,  R7"
        "flt  R0,  1.0"
        "jf   R0,  _wall_while_loop_end"

        //Check fast clipping
        "mov  R1,   {fastClippingPointer}"
        "iadd R1,   R11"
        "mov  R0,   [R1]"
        "jt   R0,   _wall_while_loop_iterators"
        "mov  [R1], R1"

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

        "out  GPU_DrawingPointX,  R11"

        //Get bottom clipping
        "mov  R12, [R13]"
        "iadd R13, 1"


        //Check if bottom is clipped
        "mov  R0,  R9"
        "mov  R1,  R12"
        "cif  R1"
        "fgt  R0,  R1"
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



        "_wall_while_loop_iterators:"

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

        "iadd R11, 1"

        "jmp  _wall_while_loop_start"

        "_wall_while_loop_end:"
    }


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
    Texture     emptyTexture;
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

    user.xPos =      0.00;
    user.yPos =      0.00;
    user.zPos =     10.00;
    user.direction = 0.0;
    user.dirSin = sin(user.direction);
    user.dirCos = cos(user.direction);

    emptyTexture.textureID = 1;
    emptyTexture.xCord  = 0;
    emptyTexture.yCord  = 0;
    emptyTexture.width  = 32;
    emptyTexture.height = 32;

    for(int i = 0; i < SCREENWIDTH; i++)
    {
        drawClip.fast[i] = false;
    }
    for(int i = 0; i < SCREENWIDTH*2; i += 2)
    {
        drawClip.full[i]   = SCREENHEIGHT - 1;
        drawClip.full[i+1] = 0;
    }
    cleanBuffer = drawClip;

    Room0.floorHeight   =  0.0;
    Room0.ceilingHeight = 32.0;
    Room0.roomHeight    = Room0.ceilingHeight - Room0.floorHeight;
    Room0.floor         = &emptyTexture;
    Room0.ceiling       = &emptyTexture;

    Room1 = Room0;

    wall0.xPos        =   48.0;
    wall0.yPos        =   96.0;
    wall0.dx          =   80.0;
    wall0.dy          =    0.0;
    wall0.length      = sqrt(wall0.dx * wall0.dx + wall0.dy * wall0.dy);
    wall0.sectorRight = &Room0;
    wall0.sectorRight = &Room1;
    wall0.bottom      = &emptyTexture;
    wall0.middle      = &emptyTexture;
    wall0.top         = &emptyTexture;
    wall0.yOffset     =    0.0;
    wall0.xOffset     =   48.0;

    wall1.xPos        =  128.0;
    wall1.yPos        =   96.0;
    wall1.dx          =    0.0;
    wall1.dy          =  -96.0;
    wall1.length      = sqrt(wall1.dx * wall1.dx + wall1.dy * wall1.dy);
    wall1.sectorRight = &Room0;
    wall1.sectorRight = &Room1;
    wall1.bottom      = &emptyTexture;
    wall1.middle      = &emptyTexture;
    wall1.top         = &emptyTexture;
    wall1.yOffset     =    0.0;
    wall1.xOffset     =    0.0;

    wall2.xPos        =  128.0;
    wall2.yPos        =    0.0;
    wall2.dx          =   -8.0;
    wall2.dy          =    0.0;
    wall2.length      = sqrt(wall2.dx * wall2.dx + wall2.dy * wall2.dy);
    wall2.sectorRight = &Room0;
    wall2.sectorRight = &Room1;
    wall2.bottom      = &emptyTexture;
    wall2.middle      = &emptyTexture;
    wall2.top         = &emptyTexture;
    wall2.yOffset     =    0.0;
    wall2.xOffset     =    0.0;

    wall3.xPos        =   76.8;
    wall3.yPos        =   57.6;
    wall3.dx          =  -19.2;
    wall3.dy          =   25.6;
    wall3.length      = sqrt(wall3.dx * wall3.dx + wall3.dy * wall3.dy);
    wall3.sectorRight = &Room0;
    wall3.sectorRight = &Room1;
    wall3.bottom      = &emptyTexture;
    wall3.middle      = &emptyTexture;
    wall3.top         = &emptyTexture;
    wall3.yOffset     =    0.0;
    wall3.xOffset     =    0.0;

    wall4.xPos        =  120.0;
    wall4.yPos        =    0.0;
    wall4.dx          = -120.0;
    wall4.dy          =    0.0;
    wall4.length      = sqrt(wall4.dx * wall4.dx + wall4.dy * wall4.dy);
    wall4.sectorRight = &Room0;
    wall4.sectorRight = &Room1;
    wall4.bottom      = &emptyTexture;
    wall4.middle      = &emptyTexture;
    wall4.top         = &emptyTexture;
    wall4.yOffset     =    0.0;
    wall4.xOffset     =    8.0;

    wall5.xPos        =   51.2;
    wall5.yPos        =   38.4;
    wall5.dx          =   25.6;
    wall5.dy          =   19.2;
    wall5.length      = sqrt(wall5.dx * wall5.dx + wall5.dy * wall5.dy);
    wall5.sectorRight = &Room0;
    wall5.sectorRight = &Room1;
    wall5.bottom      = &emptyTexture;
    wall5.middle      = &emptyTexture;
    wall5.top         = &emptyTexture;
    wall5.yOffset     =    0.0;
    wall5.xOffset     =    0.0;

    wall6.xPos        =    0.0;
    wall6.yPos        =    0.0;
    wall6.dx          =    0.0;
    wall6.dy          =   96.0;
    wall6.length      = sqrt(wall6.dx * wall6.dx + wall6.dy * wall6.dy);
    wall6.sectorRight = &Room0;
    wall6.sectorRight = &Room1;
    wall6.bottom      = &emptyTexture;
    wall6.middle      = &emptyTexture;
    wall6.top         = &emptyTexture;
    wall6.yOffset     =    0.0;
    wall6.xOffset     =    0.0;

    wall7.xPos        =    0.0;
    wall7.yPos        =   96.0;
    wall7.dx          =    8.0;
    wall7.dy          =    0.0;
    wall7.length      = sqrt(wall7.dx * wall7.dx + wall7.dy * wall7.dy);
    wall7.sectorRight = &Room0;
    wall7.sectorRight = &Room1;
    wall7.bottom      = &emptyTexture;
    wall7.middle      = &emptyTexture;
    wall7.top         = &emptyTexture;
    wall7.yOffset     =    0.0;
    wall7.xOffset     =    0.0;

    wall8.xPos        =   32.0;
    wall8.yPos        =   64.0;
    wall8.dx          =   19.2;
    wall8.dy          =  -25.6;
    wall8.length      = sqrt(wall8.dx * wall8.dx + wall8.dy * wall8.dy);
    wall8.sectorRight = &Room0;
    wall8.sectorRight = &Room1;
    wall8.bottom      = &emptyTexture;
    wall8.middle      = &emptyTexture;
    wall8.top         = &emptyTexture;
    wall8.yOffset     =    0.0;
    wall8.xOffset     =    0.0;

    wall9.xPos        =    8.0;
    wall9.yPos        =   96.0;
    wall9.dx          =   40.2;
    wall9.dy          =    0.0;
    wall9.length      = sqrt(wall9.dx * wall9.dx + wall9.dy * wall9.dy);
    wall9.sectorRight = &Room0;
    wall9.sectorRight = &Room1;
    wall9.bottom      = &emptyTexture;
    wall9.middle      = &emptyTexture;
    wall9.top         = &emptyTexture;
    wall9.yOffset     =    0.0;
    wall9.xOffset     =    8.0;

    wall10.xPos        =   57.6;
    wall10.yPos        =   83.2;
    wall10.dx          =  -25.6;
    wall10.dy          =  -19.2;
    wall10.length      = sqrt(wall10.dx * wall10.dx + wall10.dy * wall10.dy);
    wall10.sectorRight = &Room0;
    wall10.sectorRight = &Room1;
    wall10.bottom      = &emptyTexture;
    wall10.middle      = &emptyTexture;
    wall10.top         = &emptyTexture;
    wall10.yOffset     =    0.0;
    wall10.xOffset     =    0.0;

    Segment*[5] leaf0List = {&wall0, &wall1, &wall2, &wall3, NULL};
    leaf0.segList = &(leaf0List[0]);

    Segment*[3] leaf1List = {&wall4, &wall5, NULL};
    leaf1.segList = &(leaf1List[0]);

    Segment*[4] leaf2List = {&wall6, &wall7, &wall8, NULL};
    leaf2.segList = &(leaf2List[0]);

    Segment*[3] leaf3List = {&wall9, &wall10, NULL};
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
        clear_screen(color_gray);
        //drawSegment(&drawClip, &wall0, &user);
        //drawSegment(&drawClip, &wall1, &user);
        //drawSegment(&drawClip, &wall2, &user);
        //drawSegment(&drawClip, &wall3, &user);
        //drawSegment(&drawClip, &wall4, &user);
        //drawSegment(&drawClip, &wall5, &user);
        /*drawBspLeaf(
            &drawClip,
            locateBsp(&rootNode, user.xPos, user.yPos)->leaf,
            &user
        );*/
        bspRender(&drawClip, &rootNode, &user);
        //drawBspLeaf(&drawClip, &leaf0, &user);
        //drawBspLeaf(&drawClip, &leaf1, &user);
        //drawBspLeaf(&drawClip, &leaf2, &user);
        //drawBspLeaf(&drawClip, &leaf3, &user);

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
        user.zPos =  10.0 + cos((float)TIME / 30.0);

        drawClip = cleanBuffer;

        TIME++;
        end_frame();
    }

}
