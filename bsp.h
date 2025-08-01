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

struct RayHit
{
    float    xPos;
    float    yPos;
    float    zPos;
    Segment* wall;
    bool     side;
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

float* rayIntersect(Ray* ray1, Ray* ray2)
{
    float[2] result;

    float xPos;
    float yPos;

    float crossA = ray1->dx * ray2->dy;
    float crossB = ray1->dy * ray2->dx;
    float dotA   = ray1->dx * ray2->dx;
    float dotB   = ray1->dy * ray2->dy;

    xPos = (ray1->xPos * crossB) - (ray2->xPos * crossA)
         + (ray2->yPos * dotA)   - (ray1->yPos * dotA);

    yPos = (ray1->yPos * crossA) - (ray2->yPos * crossB)
         + (ray2->xPos * dotB)   - (ray1->xPos * dotB);


    if(crossA != crossB) //Result is real
    {
        xPos /= crossB - crossA;

        yPos /= crossA - crossB;

        if(xPos * ray1->dx < ray1->xPos * ray1->dx) //Behind ray1 in the x
            return NULL;

        if(yPos * ray1->dy < ray1->yPos * ray1->dy) //Behind ray1 in the y
            return NULL;

        if(xPos * ray2->dx < ray2->xPos * ray2->dx) //Behind ray2 in the x
            return NULL;

        if(yPos * ray2->dy < ray2->yPos * ray2->dy) //Behind ray2 in the y
            return NULL;

        result[0] = xPos;
        result[1] = yPos;

        return &(result[0]);
    }
    else //Result is infinite
    {
        return NULL;
    }
}


RayHit* rayCastBsp (BspBranch* currentNode, Ray* ray)
{
    float infinity;
    float negInfinity;
    asm
    {
        "mov  R0, INF"
        "mov  {infinity}, R0"
        "mov  R0, NEGINF"
        "mov  {negInfinity}, R0"
    }

    int[1000] text;

    if(currentNode->leaf != NULL)
    {
        int       segSize  = sizeof(BspLeaf);
        Segment** wallList = currentNode->leaf->segList;
        Segment*  currentWall;
        float*    hitPoint;
        float     wallDistanceSQRD;
        Ray       wallRay;

        while(*wallList != NULL)
        {
            currentWall = *wallList;

            if(onRightSideSeg(currentWall, ray->xPos, ray->yPos))
            {
                wallRay.xPos = currentWall->xPos;
                wallRay.yPos = currentWall->yPos;
                wallRay.dx   = currentWall->dx;
                wallRay.dy   = currentWall->dy;

                hitPoint = rayIntersect(&wallRay, ray);
                if(hitPoint != NULL)
                {
                    float xPos = *(hitPoint);
                    float yPos = *(hitPoint + FLOATSIZE);

                    wallDistanceSQRD = pow(xPos - wallRay.xPos, 2.0)
                                     + pow(yPos - wallRay.yPos, 2.0);

                    if(wallDistanceSQRD <= pow(currentWall->length, 2.0))
                    {
                        RayHit returnData;
                        returnData.xPos = xPos;
                        returnData.yPos = yPos;
                        returnData.zPos = ray->zPos;
                        returnData.wall = currentWall;

                        return &returnData;
                    }
                }
            }

            wallList += segSize;
        }
    }

    bool side = onRightSideBranch(currentNode, ray->xPos, ray->yPos);
    RayHit* returnHit = NULL;
    BspBranch* tempNode;

    if(side == RIGHT)
    {

        tempNode = currentNode->rightNode;
        if(tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray);
        }

        if(returnHit != NULL && tempNode != NULL)
        {
                returnHit = rayCastBsp(tempNode, ray);
        }
    }
    else
    {
        tempNode = currentNode->leftNode;
        if(tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray);
        }

        tempNode = currentNode->rightNode;
        if(returnHit != NULL && tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray);
        }
    }

    return returnHit;

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
