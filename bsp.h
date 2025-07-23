#ifndef BSPH
#define BSPH

#include "defines.h"
#include "general.h"

struct Texture
{
    int textureID;
    int width;
    int height;

};

struct SkyBox
{
    Texture* texture;
    float    rotation;
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
    bool     isSkyBox;
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
    float      angle;
    bool       BranchSide;
    BspBranch* parentNode;
    BspBranch* rightNode;
    BspBranch* leftNode;
    BspLeaf*   leaf;
};

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

void areBranchesVisable(BspBranch* branch, Player* pov, bool* truthValues)
{
    float relativeAngle = branch->angle - pov->direction;

    if(cos(2.0 * relativeAngle) >= 0) //Parallel ish
    {
        truthValues[0] = true;
        truthValues[1] = true;
        return;
    }
    else //Perpendicular ish
    {
        if(sin(relativeAngle) >= 0) //Left facing
        {
            if(onRightSideBranch(branch, pov->xPos, pov->yPos) == RIGHT)
            {
                truthValues[0] = false;
                truthValues[1] = true;
                return;
            }
            else
            {
                truthValues[0] = true;
                truthValues[1] = true;
                return;
            }
        }
        else //Right facing
        {
            if(onRightSideBranch(branch, pov->xPos, pov->yPos) == RIGHT)
            {
                truthValues[0] = true;
                truthValues[1] = true;
                return;
            }
            else
            {
                truthValues[0] = true;
                truthValues[1] = false;
                return;
            }
        }
    }
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


#endif
