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

    float   fTest;
    int     iTest;
    void*   pTest;

    int         TIME;
    Player      user;
    Texture     wallTexture;
    Texture     testTexture;
    Texture     skyTexture;
    SkyBox      plainSky;
    FrameBuffer drawClip;
    FrameBuffer cleanBuffer;
    int[SCREENWIDTH] filledFastClipping;
    Sector      Room0;
    Sector      Room1;
    Sector      Room2;
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
    Segment     testwall1;
    Segment     testwall2;
    Segment*[5] leaf0List = {&wall3, &wall0, &wall1, &wall2, NULL};
    Segment*[3] leaf1List = {&wall5, &wall4, NULL};
    Segment*[4] leaf2List = {&wall8, &wall6, &wall7, NULL};
    Segment*[3] leaf3List = {&wall10, &wall9, NULL};
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

    int[732] text;

    user.xPos =      5.00;
    user.yPos =      5.00;
    user.zPos =     16.00;
    user.xSpeed =    0.00;
    user.ySpeed =    0.00;
    user.zSpeed =    0.00;
    user.direction = 0.00;
    user.dirSin = sin(user.direction);
    user.dirCos = cos(user.direction);

    wallTexture.textureID = 1;
    wallTexture.width  = 32;
    wallTexture.height = 32;

    testTexture.textureID = 0;
    testTexture.width  = 32;
    testTexture.height = 32;

    skyTexture.textureID = 3;
    skyTexture.width  = 400;
    skyTexture.height = 100;

    plainSky.texture = &skyTexture;
    plainSky.rotation = 0.1;

    for(int i = 0; i < SCREENWIDTH; i++)
    {
        drawClip.fast[i] = false;
        filledFastClipping[i] = true;
    }
    for(int i = 0; i < SCREENWIDTH*2; i += 2)
    {
        drawClip.full[i]   = SCREENHEIGHT;
        drawClip.full[i+1] = 0;
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

    Room2.floorHeight   =  4.0;
    Room2.ceilingHeight = 28.0;
    Room2.floorColor    = 0xFFFFFFFF;
    Room2.ceilingColor  = 0xFFFFFFFF;

    wall0.Name        = "wall0";
    wall0.xPos        =   48.0;
    wall0.yPos        =   96.0;
    wall0.dx          =   80.0;
    wall0.dy          =    0.0;
    wall0.length      = sqrt(wall0.dx * wall0.dx + wall0.dy * wall0.dy);
    wall0.isPortal    = false;
    wall0.isSkyBox    = false;
    wall0.sectorRight = &Room0;
    wall0.sectorLeft  = &Room2;
    wall0.bottom      = &wallTexture;
    wall0.middle      = &wallTexture;
    wall0.top         = &wallTexture;
    wall0.yOffset     =    0.0;
    wall0.xOffset     =   48.0;

    wall1.Name        = "wall1";
    wall1.xPos        =  128.0;
    wall1.yPos        =   96.0;
    wall1.dx          =    0.0;
    wall1.dy          =  -96.0;
    wall1.length      = sqrt(wall1.dx * wall1.dx + wall1.dy * wall1.dy);
    wall1.isPortal    = true;
    wall1.isSkyBox    = true;
    wall1.sectorRight = &Room0;
    wall1.sectorLeft  = &Room2;
    wall1.bottom      = &wallTexture;
    wall1.middle      = &wallTexture;
    wall1.top         = &wallTexture;
    wall1.yOffset     =    0.0;
    wall1.xOffset     =    0.0;

    wall2.Name        = "wall2";
    wall2.xPos        =  128.0;
    wall2.yPos        =    0.0;
    wall2.dx          =   -8.0;
    wall2.dy          =    0.0;
    wall2.length      = sqrt(wall2.dx * wall2.dx + wall2.dy * wall2.dy);
    wall2.isPortal    = false;
    wall2.isSkyBox    = false;
    wall2.sectorRight = &Room0;
    wall2.sectorLeft  = &Room2;
    wall2.bottom      = &wallTexture;
    wall2.middle      = &wallTexture;
    wall2.top         = &wallTexture;
    wall2.yOffset     =    0.0;
    wall2.xOffset     =    0.0;

    wall3.Name        = "wall3";
    wall3.xPos        =   76.8;
    wall3.yPos        =   57.6;
    wall3.dx          =  -19.2;
    wall3.dy          =   25.6;
    wall3.length      = sqrt(wall3.dx * wall3.dx + wall3.dy * wall3.dy);
    wall3.isPortal    = true;
    wall3.isSkyBox    = false;
    wall3.sectorRight = &Room0;
    wall3.sectorLeft  = &Room1;
    wall3.bottom      = &wallTexture;
    wall3.middle      = &wallTexture;
    wall3.top         = &testTexture;
    wall3.yOffset     =    0.0;
    wall3.xOffset     =    0.0;

    wall4.Name        = "wall4";
    wall4.xPos        =  120.0;
    wall4.yPos        =    0.0;
    wall4.dx          = -120.0;
    wall4.dy          =    0.0;
    wall4.length      = sqrt(wall4.dx * wall4.dx + wall4.dy * wall4.dy);
    wall4.isPortal    = false;
    wall4.isSkyBox    = false;
    wall4.sectorRight = &Room0;
    wall4.sectorLeft  = &Room2;
    wall4.bottom      = &wallTexture;
    wall4.middle      = &wallTexture;
    wall4.top         = &wallTexture;
    wall4.yOffset     =    0.0;
    wall4.xOffset     =    8.0;

    wall5.Name        = "wall5";
    wall5.xPos        =   51.2;
    wall5.yPos        =   38.4;
    wall5.dx          =   25.6;
    wall5.dy          =   19.2;
    wall5.length      = sqrt(wall5.dx * wall5.dx + wall5.dy * wall5.dy);
    wall5.isPortal    = true;
    wall5.isSkyBox    = false;
    wall5.sectorRight = &Room0;
    wall5.sectorLeft  = &Room1;
    wall5.bottom      = &wallTexture;
    wall5.middle      = &wallTexture;
    wall5.top         = &wallTexture;
    wall5.yOffset     =    0.0;
    wall5.xOffset     =    0.0;

    wall6.Name        = "wall6";
    wall6.xPos        =    0.0;
    wall6.yPos        =    0.0;
    wall6.dx          =    0.0;
    wall6.dy          =   96.0;
    wall6.length      = sqrt(wall6.dx * wall6.dx + wall6.dy * wall6.dy);
    wall6.isPortal    = false;
    wall6.isSkyBox    = true;
    wall6.sectorRight = &Room0;
    wall6.sectorLeft  = &Room2;
    wall6.bottom      = &wallTexture;
    wall6.middle      = &wallTexture;
    wall6.top         = &wallTexture;
    wall6.yOffset     =    0.0;
    wall6.xOffset     =    0.0;

    wall7.Name        = "wall7";
    wall7.xPos        =    0.0;
    wall7.yPos        =   96.0;
    wall7.dx          =    8.0;
    wall7.dy          =    0.0;
    wall7.length      = sqrt(wall7.dx * wall7.dx + wall7.dy * wall7.dy);
    wall7.isPortal    = false;
    wall7.isSkyBox    = false;
    wall7.sectorRight = &Room0;
    wall7.sectorLeft  = &Room2;
    wall7.bottom      = &wallTexture;
    wall7.middle      = &wallTexture;
    wall7.top         = &wallTexture;
    wall7.yOffset     =    0.0;
    wall7.xOffset     =    0.0;

    wall8.Name        = "wall8";
    wall8.xPos        =   32.0;
    wall8.yPos        =   64.0;
    wall8.dx          =   19.2;
    wall8.dy          =  -25.6;
    wall8.length      = sqrt(wall8.dx * wall8.dx + wall8.dy * wall8.dy);
    wall8.isPortal    = true;
    wall8.isSkyBox    = false;
    wall8.sectorRight = &Room0;
    wall8.sectorLeft  = &Room1;
    wall8.bottom      = &wallTexture;
    wall8.middle      = &wallTexture;
    wall8.top         = &wallTexture;
    wall8.yOffset     =    0.0;
    wall8.xOffset     =    0.0;

    wall9.Name        = "wall9";
    wall9.xPos        =    8.0;
    wall9.yPos        =   96.0;
    wall9.dx          =   40.0;
    wall9.dy          =    0.0;
    wall9.length      = sqrt(wall9.dx * wall9.dx + wall9.dy * wall9.dy);
    wall9.isPortal    = false;
    wall9.isSkyBox    = false;
    wall9.sectorRight = &Room0;
    wall9.sectorLeft  = &Room2;
    wall9.bottom      = &wallTexture;
    wall9.middle      = &wallTexture;
    wall9.top         = &wallTexture;
    wall9.yOffset     =    0.0;
    wall9.xOffset     =    8.0;

    wall10.Name        = "wall10";
    wall10.xPos        =   57.6;
    wall10.yPos        =   83.2;
    wall10.dx          =  -25.6;
    wall10.dy          =  -19.2;
    wall10.length      = sqrt(wall10.dx * wall10.dx + wall10.dy * wall10.dy);
    wall10.isPortal    = true;
    wall10.isSkyBox    = false;
    wall10.sectorRight = &Room0;
    wall10.sectorLeft  = &Room1;
    wall10.bottom      = &wallTexture;
    wall10.middle      = &wallTexture;
    wall10.top         = &wallTexture;
    wall10.yOffset     =    0.0;
    wall10.xOffset     =    0.0;

    testwall1.Name        = "testwall1";
    testwall1.xPos        =  138.0;
    testwall1.yPos        =   96.0;
    testwall1.dx          =    0.0;
    testwall1.dy          =  -96.0;
    testwall1.length      = sqrt(wall1.dx * wall1.dx + wall1.dy * wall1.dy);
    testwall1.isPortal    = false;
    testwall1.isSkyBox    = false;
    testwall1.sectorRight = &Room0;
    testwall1.sectorLeft  = &Room2;
    testwall1.bottom      = &wallTexture;
    testwall1.middle      = &wallTexture;
    testwall1.top         = &wallTexture;
    testwall1.yOffset     =    0.0;
    testwall1.xOffset     =    0.0;

    testwall2.Name        = "testwall2";
    testwall2.xPos        =  -10.0;
    testwall2.yPos        =    0.0;
    testwall2.dx          =    0.0;
    testwall2.dy          =   96.0;
    testwall2.length      = sqrt(wall1.dx * wall1.dx + wall1.dy * wall1.dy);
    testwall2.isPortal    = false;
    testwall2.isSkyBox    = false;
    testwall2.sectorRight = &Room0;
    testwall2.sectorLeft  = &Room2;
    testwall2.bottom      = &wallTexture;
    testwall2.middle      = &wallTexture;
    testwall2.top         = &wallTexture;
    testwall2.yOffset     =    0.0;
    testwall2.xOffset     =    0.0;

    leaf0.segList = &(leaf0List[0]);

    leaf1.segList = &(leaf1List[0]);

    leaf2.segList = &(leaf2List[0]);

    leaf3.segList = &(leaf3List[0]);

    rootNode.Name       = "root";
    rootNode.HyperX     =   76.8;
    rootNode.HyperY     =   57.6;
    rootNode.HyperDx    =  -19.2;
    rootNode.HyperDy    =   25.6;
    rootNode.angle      = atan2(rootNode.HyperDy, rootNode.HyperDx);
    rootNode.BranchSide = NULL;
    rootNode.rightNode  = &node0;
    rootNode.leftNode   = &node1;
    rootNode.parentNode = NULL;
    rootNode.leaf       = NULL;

    node0.Name       = "node0";
    node0.HyperX     = NULL;
    node0.HyperY     = NULL;
    node0.HyperDx    = NULL;
    node0.HyperDy    = NULL;
    node0.angle      = atan2(node0.HyperDy, node0.HyperDx);
    node0.BranchSide = RIGHT;
    node0.rightNode  = NULL;
    node0.leftNode   = NULL;
    node0.parentNode = &rootNode;
    node0.leaf       = &leaf0;

    node1.Name       = "node1";
    node1.HyperX     =   51.2;
    node1.HyperY     =   38.4;
    node1.HyperDx    =   25.6;
    node1.HyperDy    =   19.2;
    node1.angle      = atan2(node1.HyperDy, node1.HyperDx);
    node1.BranchSide = LEFT;
    node1.rightNode  = &node2;
    node1.leftNode   = &node3;
    node1.parentNode = &rootNode;
    node1.leaf       = NULL;

    node2.Name       = "node2";
    node2.HyperX     = NULL;
    node2.HyperY     = NULL;
    node2.HyperDx    = NULL;
    node2.HyperDy    = NULL;
    node2.angle      = atan2(node2.HyperDy, node2.HyperDx);
    node2.BranchSide = RIGHT;
    node2.rightNode  = NULL;
    node2.leftNode   = NULL;
    node2.parentNode = &node1;
    node2.leaf       = &leaf1;

    node3.Name       = "node3";
    node3.HyperX     =   32.0;
    node3.HyperY     =   64.0;
    node3.HyperDx    =   19.2;
    node3.HyperDy    =  -25.6;
    node3.angle      = atan2(node3.HyperDy, node3.HyperDx);
    node3.BranchSide = LEFT;
    node3.rightNode  = &node4;
    node3.leftNode   = &node5;
    node3.parentNode = &node1;
    node3.leaf       = NULL;

    node4.Name       = "node4";
    node4.HyperX     = NULL;
    node4.HyperY     = NULL;
    node4.HyperDx    = NULL;
    node4.HyperDy    = NULL;
    node4.angle      = atan2(node4.HyperDy, node4.HyperDx);
    node4.BranchSide = RIGHT;
    node4.rightNode  = NULL;
    node4.leftNode   = NULL;
    node4.parentNode = &node3;
    node4.leaf       = &leaf2;

    node5.Name       = "node5";
    node5.HyperX     = NULL;
    node5.HyperY     = NULL;
    node5.HyperDx    = NULL;
    node5.HyperDy    = NULL;
    node5.angle      = atan2(node5.HyperDy, node5.HyperDx);
    node5.BranchSide = LEFT;
    node5.rightNode  = NULL;
    node5.leftNode   = NULL;
    node5.parentNode = &node3;
    node5.leaf       = &leaf3;

    while(true)
    {
        clear_screen(color_black);
        drawSkyBox(&plainSky, &user);

        print_at(0,  320, "X:");
        ftoa(user.xPos, text);
        print_at(20, 320, text);
        ftoa(user.yPos, text);
        print_at(0,  340, "Y:");
        print_at(20, 340, text);

        playerMovement(&user, &rootNode);

        user.zPos =  16.0 + 1.5*sin((float)TIME / 30.0);

        wall3.yOffset = 1.0 + 1.0*sin((float)TIME / 15.0);
        Room1.floorHeight = 14.0 + 14.0*sin((float)TIME / 120.0);
        Room1.ceilingHeight = 16.0 + 14.0*sin((float)TIME / 120.0);
        Room2.floorHeight = 2.0 + 5.0*sin((float)TIME / 60.0);
        Room2.ceilingHeight = 28.0 + 5.0*sin((float)TIME / 55.0);
        wall8.xOffset = 128.0 + 128.0*sin((float)TIME / 360.0);

        bspRender(filledFastClipping, &drawClip, &rootNode, &user);
        //bspRender(filledFastClipping, &drawClip, &node5, &user);
        //drawSegment(&drawClip, &testwall1, &user);
        //drawSegment(&drawClip, &testwall2, &user);

        drawClip = cleanBuffer;

        inputWait();
        if(gamepad_button_l() < 0 || gamepad_button_r() == 1)
        {
            TIME++;
        }

        //asm{"call _debugregs"}
        end_frame();
    }

}
