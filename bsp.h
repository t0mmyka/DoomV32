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
    int*     Name;
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
    int*       Name;
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

bool onLeftSideSeg(Segment* seg, float x, float y)
{
    float dx = x - seg->xPos;
    float dy = y - seg->yPos;

    float left  = dx * seg->dy;
    float right = dy * seg->dx;
    if(right > left)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//NOTE: ray2 hits ray1
float* rayIntersect(Ray* ray1, Ray* ray2)
{
    float[2] result;

    float xPos;
    float yPos;

    float R1dx = ray1->dx;
    float R1dy = ray1->dy;
    float R2dx = ray2->dx;
    float R2dy = ray2->dy;

    normalize(&R1dx, &R1dy);
    normalize(&R2dx, &R2dy);

    float crossA = R1dx * R2dy;
    float crossB = R1dy * R2dx;
    float dotA   = R1dx * R2dx;
    float dotB   = R1dy * R2dy;

    xPos = (ray1->xPos * crossB) - (ray2->xPos * crossA)
         + (ray2->yPos * dotA)   - (ray1->yPos * dotA);

    yPos = (ray1->yPos * crossA) - (ray2->yPos * crossB)
         + (ray2->xPos * dotB)   - (ray1->xPos * dotB);


    if(crossA != crossB) //Single hit
    {
        xPos /= crossB - crossA;

        yPos /= crossA - crossB;

        if(xPos * R1dx < ray1->xPos * R1dx) //Behind ray1 in the x
            return NULL;

        if(yPos * R1dy < ray1->yPos * R1dy) //Behind ray1 in the y
            return NULL;

        if(xPos * R2dx < ray2->xPos * R2dx) //Behind ray2 in the x
            return NULL;

        if(yPos * R2dy < ray2->yPos * R2dy) //Behind ray2 in the y
            return NULL;

        result[0] = xPos;
        result[1] = yPos;

        return &(result[0]);
    }
    else //Parallel
    {
        if(xPos == 0.0 && yPos == 0.0) //Hit
        {
            if(ray2->xPos * R1dx < ray1->xPos * R1dx)
            {
                result[0] = ray1->xPos;
                result[1] = ray1->yPos;
            }
            else if(ray2->yPos * R1dy < ray1->yPos * R1dy)
            {
                result[0] = ray1->xPos;
                result[1] = ray1->yPos;
            }
            else
            {
                result[0] = ray2->xPos;
                result[1] = ray2->yPos;
            }

            return &(result[0]);
        }
        else //No Hit
        {
            return NULL;
        }
    }
}


RayHit* rayCastBsp (BspBranch* currentNode, Ray* ray, RayHit* returnData)
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

    RayHit* returnHit = NULL;

    if(currentNode->leaf != NULL)
    {
        int       segSize  = sizeof(Segment*);
        Segment** wallList = currentNode->leaf->segList;
        Segment*  currentWall;
        float*    hitPoint;
        float     wallDistance;
        Ray       wallRay;

        while(*wallList != NULL)
        {
            currentWall = *wallList;

            if(!onLeftSideSeg(currentWall, ray->xPos, ray->yPos))
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

                    wallDistance = dist(xPos - wallRay.xPos, yPos - wallRay.yPos);

                    if(wallDistance <= currentWall->length)
                    {
                        bool newHit = false;

                        if(returnHit == NULL)
                        {
                            newHit = true;
                            returnHit = returnData;
                        }
                        else if(
                            dist(xPos - ray->xPos, yPos - ray->yPos) >
                            dist(returnData->xPos - ray->xPos, returnData->yPos - ray->yPos)
                        )
                        {
                            newHit = true;
                        }

                        if(newHit == true)
                        {
                            returnData->xPos = xPos;
                            returnData->yPos = yPos;
                            returnData->zPos = ray->zPos;
                            returnData->wall = currentWall;
                        }
                    }
                }
            }

            wallList += segSize;
        }
        if(returnHit != NULL)
            return returnData;
    }

    bool side = onRightSideBranch(currentNode, ray->xPos, ray->yPos);

    BspBranch* tempNode;

    if(side == RIGHT)
    {

        tempNode = currentNode->rightNode;
        if(tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray, returnData);
        }

        tempNode = currentNode->leftNode;
        if(returnHit == NULL && tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray, returnData);
        }
    }
    else
    {
        tempNode = currentNode->leftNode;
        if(tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray, returnData);
        }

        tempNode = currentNode->rightNode;
        if(returnHit == NULL && tempNode != NULL)
        {
            returnHit = rayCastBsp(tempNode, ray, returnData);
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

void playerMovement(Player* person, BspBranch* collisionMap)
{
    int[50] text;

    //Turn player
    if(gamepad_button_b() > 0)
        person->direction += 0.001 * gamepad_button_b();
    if(gamepad_button_a() > 0)
        person->direction -= 0.001 * gamepad_button_a();

    person->dirCos = cos(person->direction);
    person->dirSin = sin(person->direction);

    //Apply friction
    person->xSpeed *= 1.0 - PFRICTION;
    person->ySpeed *= 1.0 - PFRICTION;

    //Apply inputs
    if(gamepad_left() > 0)
    {
        person->xSpeed -= (float)min(gamepad_left(), 5)
                     * person->dirSin * PACCELERAION;
        person->ySpeed += (float)min(gamepad_left(), 5)
                     * person->dirCos * PACCELERAION;
    }
    if(gamepad_right() > 0)
    {
        person->xSpeed += (float)min(gamepad_right(), 5)
                     * person->dirSin * PACCELERAION;
        person->ySpeed -= (float)min(gamepad_right(), 5)
                     * person->dirCos * PACCELERAION;
    }
    if(gamepad_up() > 0)
    {
        person->xSpeed += (float)min(gamepad_up(), 5)
                     * person->dirCos * PACCELERAION;
        person->ySpeed += (float)min(gamepad_up(), 5)
                     * person->dirSin * PACCELERAION;
    }
    if(gamepad_down() > 0)
    {
        person->xSpeed -= (float)min(gamepad_down(), 5)
                     * person->dirCos * PACCELERAION;
        person->ySpeed -= (float)min(gamepad_down(), 5)
                     * person->dirSin * PACCELERAION;
    }

    if(gamepad_button_l() > 0)
    {
        person->xSpeed = 0.0;
        person->ySpeed = 0.0;
    }



    //Limit speed
    if(dist(person->xSpeed, person->ySpeed) > PMAXSPEED)
    {
        float angle = atan2(person->ySpeed, person->xSpeed);
        person->xSpeed = PMAXSPEED * cos(angle);
        person->ySpeed = PMAXSPEED * sin(angle);
    }

    Ray     motion;
    RayHit  hitData;
    RayHit* intersect;

    if(person->xSpeed != 0.0 || person->ySpeed != 0.0)
    {
        float speed = dist(person->xSpeed, person->ySpeed);

        motion.dx = person->xSpeed;
        motion.dy = person->ySpeed;
        motion.dz = person->zSpeed;
        motion.xPos = person->xPos;
        motion.yPos = person->yPos;
        motion.zPos = person->zPos;

        intersect = rayCastBsp(collisionMap , &motion, &hitData);

        bool hit = false;

        if(intersect != NULL)
        {
            float hitDistance = dist(
                intersect->xPos - person->xPos,
                intersect->yPos - person->yPos
            );

            print_at(240, 320, "D:");
            ftoa(hitDistance, text);
            print_at(260, 320, text);
            ftoa(speed, text);
            print_at(240, 340, "S:");
            print_at(260, 340, text);

            if(hitDistance <= speed)
            {
                hit = true;
            }
        }

        if(hit == true)
        {
            float newPosX = intersect->xPos;
            float newPosY = intersect->yPos;

            Segment* wall = intersect->wall;

            float wallAngle = atan2(wall->dy, wall->dx);
            float rayAngle  = atan2(person->ySpeed, person->xSpeed);

            newPosX -= (cos(rayAngle) / sin(rayAngle - wallAngle)) * MOVEPADDING;
            newPosY -= (sin(rayAngle) / sin(rayAngle - wallAngle)) * MOVEPADDING;

            person->xPos = newPosX;
            person->yPos = newPosY;
            person->xSpeed = 0.0;
            person->ySpeed = 0.0;
        }
        else
        {
            person->xPos += person->xSpeed;
            person->yPos += person->ySpeed;
            person->zPos += person->zSpeed;
        }
    }
    else
    {
        motion.dx = person->dirCos;
        motion.dy = person->dirSin;
        motion.dz = 0;
        motion.xPos = person->xPos;
        motion.yPos = person->yPos;
        motion.zPos = person->zPos;

        intersect = rayCastBsp(collisionMap , &motion, &hitData);

    }
    if(intersect != NULL)
    {
        print_at(120, 320, "X:");
        ftoa(intersect->xPos, text);
        print_at(140, 320, text);
        ftoa(intersect->yPos, text);
        print_at(120, 340, "Y:");
        print_at(140, 340, text);
    }
}


#endif
