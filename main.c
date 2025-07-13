#include "video.h"
#include "math.h"
#include "string.h"
#include "time.h"
#include "misc.h"
#include "input.h"
#include "defines.h"
#include "general.h"
#include "bsp.h"
#include "render.h"


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

    user.xPos =     50.00;
    user.yPos =     30.00;
    user.zPos =     16.00;
    user.direction = 1.4;
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
    Room0.ceilingHeight = 30.0;
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
        clear_screen(color_orange);

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
        user.zPos =  16.0 + 1.5*sin((float)TIME / 30.0);
        wall3.yOffset = 1.0 + 1.0*sin((float)TIME / 15.0);
        Room1.floorHeight = 14.0 + 14.0*sin((float)TIME / 120.0);
        Room1.ceilingHeight = 16.0 + 14.0*sin((float)TIME / 120.0);

        bspRender(&drawClip, &rootNode, &user);

        drawClip = cleanBuffer;

        if(gamepad_button_l() < 0 || gamepad_button_r() == 1)
        {
            TIME++;
        }
        inputWait();
        end_frame();
    }

}
