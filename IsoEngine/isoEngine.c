#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "isoEngine.h"
#include "../logger.h"
#include "../renderer.h"

void setupRect(SDL_Rect *rect,int x,int y,int w,int h)
{
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

isoEngineT *isoEngineNewIsoEngine()
{
    isoEngineT *isoEngine = malloc(sizeof(struct isoEngineT));

    if(isoEngine == NULL)
    {
        writeToLog("Error in isoEngineNewIsoEngine(...): Could not allocate memory for new isoEngine!","error.txt");
        return NULL;
    }
    isoEngine->scrollX = 0;
    isoEngine->scrollY = 0;
    isoEngine->mapScrollSpeed = 6;
    isoEngine->mapScroll2Dpos.x = 0;
    isoEngine->mapScroll2Dpos.y = 0;
    isoEngine->zoomLevel = 1.0;
    isoEngine->lastTileClicked = -1;
    isoEngine->isoMap = NULL;

    setupRect(&isoEngine->mouseRect,0,0,1,1);
    isoEngine->tilePos.x = 0;
    isoEngine->tilePos.y = 0;

    return isoEngine;
}

void isoEngineFreeIsoEngine(isoEngineT *isoEngine)
{
    if(isoEngine != NULL)
    {
        if(isoEngine->isoMap!=NULL)
        {
            isoMapFreeMap(isoEngine->isoMap);
        }
        free(isoEngine);
    }
}

void isoEngineConvert2dToIso(point2DT *point)
{
    int tmpX = point->x - point->y;
    int tmpY = (point->x + point->y)*0.5;
    point->x = tmpX;
    point->y = tmpY;
}

void isoEngineConvertIsoTo2D(point2DT *point)
{
    int tmpX = (2 * point->y + point->x)*0.5;
    int tmpY = (2 * point->y - point->x)*0.5;
    point->x = tmpX;
    point->y = tmpY;
}

void isoEngineGetTileCoordinates(isoEngineT *isoEngine,point2DT *point,point2DT *point2DCoord)
{
    if(isoEngine == NULL){
        writeToLog("Error in isoEngineGetTileCoordinates(...): - isoEngine is NULL!","error.txt");
        return;
    }
    if(isoEngine->isoMap == NULL){
        writeToLog("Error in isoEngineGetTileCoordinates(...): - isoEngine->isoMap is NULL!","error.txt");
        return;
    }
    float tempX = (float)point->x / (float)isoEngine->isoMap->tileSize;
    float tempY = (float)point->y / (float)isoEngine->isoMap->tileSize;;

    //convert back to integer
    point2DCoord->x = (int)tempX;
    point2DCoord->y = (int)tempY;
}

void isoEngineConvertIsoCameraToCartesian(isoEngineT *isoEngine,point2DT *cartesianCamPos)
{
    point2DT tmpPoint;

    if(isoEngine == NULL || cartesianCamPos == NULL)
    {
        return;
    }

    tmpPoint.x = isoEngine->scrollX;
    tmpPoint.y = isoEngine->scrollY;

    isoEngineConvert2dToIso(&tmpPoint);

    tmpPoint.x = tmpPoint.x/2;

    if(tmpPoint.x<0){
        tmpPoint.x = abs(tmpPoint.x);
    }
    else if(tmpPoint.x>0)
    {
        tmpPoint.x = -abs(tmpPoint.x);
    }
    *cartesianCamPos = tmpPoint;
}

void isoEngineConvertCartesianCameraToIsometric(isoEngineT *isoEngine,point2DT *cartesianCamPos)
{
    point2DT tmpPoint;

    if(isoEngine == NULL || cartesianCamPos == NULL)
    {
        return;
    }
    tmpPoint = *cartesianCamPos;

    tmpPoint.x = (int)tmpPoint.x*2;

    if(tmpPoint.x<0){
        tmpPoint.x = abs((int)tmpPoint.x);
    }
    else if(tmpPoint.x>0)
    {
        tmpPoint.x = -abs((int)tmpPoint.x);
    }

    isoEngineConvertIsoTo2D(&tmpPoint);

    isoEngine->scrollX = (int)tmpPoint.x;
    isoEngine->scrollY = (int)tmpPoint.y;
}

void isoEngineUpdateMousePos(isoEngineT *isoEngine)
{
    SDL_GetMouseState(&isoEngine->mouseRect.x,&isoEngine->mouseRect.y);
    isoEngine->mouseRect.x = isoEngine->mouseRect.x/isoEngine->zoomLevel;
    isoEngine->mouseRect.y = isoEngine->mouseRect.y/isoEngine->zoomLevel;
}

void isoEngineScrollMapWithMouse(isoEngineT *isoEngine)
{
    int zoomEdgeX = (WINDOW_WIDTH*isoEngine->zoomLevel)-(WINDOW_WIDTH);
    int zoomEdgeY = (WINDOW_HEIGHT*isoEngine->zoomLevel)-(WINDOW_HEIGHT);

    if(isoEngine->mouseRect.x<2){
        isoEngine->mapScroll2Dpos.x-=isoEngine->mapScrollSpeed;
        isoEngineConvertCartesianCameraToIsometric(isoEngine,&isoEngine->mapScroll2Dpos);
    }
    if(isoEngine->mouseRect.x>WINDOW_WIDTH-(zoomEdgeX/isoEngine->zoomLevel)-2){
        isoEngine->mapScroll2Dpos.x+=isoEngine->mapScrollSpeed;
        isoEngineConvertCartesianCameraToIsometric(isoEngine,&isoEngine->mapScroll2Dpos);

    }
    if(isoEngine->mouseRect.y<2){
        isoEngine->mapScroll2Dpos.y+=isoEngine->mapScrollSpeed;
        isoEngineConvertCartesianCameraToIsometric(isoEngine,&isoEngine->mapScroll2Dpos);

    }
    if(isoEngine->mouseRect.y>WINDOW_HEIGHT-(zoomEdgeY/isoEngine->zoomLevel)-2){
        isoEngine->mapScroll2Dpos.y-=isoEngine->mapScrollSpeed;
        isoEngineConvertCartesianCameraToIsometric(isoEngine,&isoEngine->mapScroll2Dpos);

    }
}

void isoEngineDrawIsoMouse(isoEngineT *isoEngine)
{
    if(isoEngine == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine is NULL!","error.txt");
        return;
    }
    if(isoEngine->isoMap == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine->isoMap is NULL!","error.txt");
        return;
    }
    int modulusX = isoEngine->isoMap->tileSize*isoEngine->zoomLevel;
    int modulusY = isoEngine->isoMap->tileSize*isoEngine->zoomLevel;
    int correctX =(((int)isoEngine->mapScroll2Dpos.x)%modulusX)*2;
    int correctY = ((int)isoEngine->mapScroll2Dpos.y)%modulusY;

    isoEngine->mousePoint.x = (isoEngine->mouseRect.x/isoEngine->isoMap->tileSize) * isoEngine->isoMap->tileSize;
    isoEngine->mousePoint.y = (isoEngine->mouseRect.y/isoEngine->isoMap->tileSize) * isoEngine->isoMap->tileSize;

    //For every other x position on the map
    if(((int)isoEngine->mousePoint.x/isoEngine->isoMap->tileSize)%2){
        //Move the mouse down by half a tile so we can
        //pick isometric tiles on that row as well.
        isoEngine->mousePoint.y+=isoEngine->isoMap->tileSize*0.5;
    }
    textureRenderXYClipScale(&isoEngine->isoMap->tileSet->tilesTex[0],(isoEngine->zoomLevel*isoEngine->mousePoint.x)-correctX,
                             (isoEngine->zoomLevel*isoEngine->mousePoint.y)+correctY,&isoEngine->isoMap->tileSet->tileClipRects[0],isoEngine->zoomLevel);
}


void isoEngineDrawIsoMap(isoEngineT *isoEngine)
{
    int i,j;
    int x,y;
    int tile = 4;
    point2DT point;

    if(isoEngine==NULL){
        return;
    }
    if(isoEngine->isoMap == NULL){
        return;
    }

    int startX = -3/isoEngine->zoomLevel +(isoEngine->mapScroll2Dpos.x/isoEngine->zoomLevel/isoEngine->isoMap->tileSize)*2;
    int startY = -20/isoEngine->zoomLevel + abs((isoEngine->mapScroll2Dpos.y/isoEngine->zoomLevel/isoEngine->isoMap->tileSize))*2;
    int numTilesInWidth = ((WINDOW_WIDTH/isoEngine->isoMap->tileSize)/isoEngine->zoomLevel);
    int numTilesInHeight = ((WINDOW_HEIGHT/isoEngine->isoMap->tileSize)/isoEngine->zoomLevel)*2;

    if(isoEngine->isoMap->tileSet != NULL){

        for(i=startY;i<startY+numTilesInHeight+26;++i){
            for(j=startX;j<startX+numTilesInWidth+5;++j){

                //only draw when both x & y is equal, so we skip here
                if((j&1) != (i&1)){
                    continue;
                }
                x = (i+j)/2;
                y = (i-j)/2;

                if(x>=0 && y>=0 && x<isoEngine->isoMap->mapWidth && y<isoEngine->isoMap->mapHeight){
                    tile = isoMapGetTile(isoEngine->isoMap,x,y,0);
                    point.x = ((x*isoEngine->zoomLevel *isoEngine->isoMap->tileSize) + isoEngine->scrollX);
                    point.y = ((y*isoEngine->zoomLevel *isoEngine->isoMap->tileSize) + isoEngine->scrollY);
                    isoEngineConvert2dToIso(&point);
                    textureRenderXYClipScale(isoEngine->isoMap->tileSet->tilesTex,point.x,point.y,
                                             &isoEngine->isoMap->tileSet->tileClipRects[tile],isoEngine->zoomLevel);
                }
            }
        }
    }
}

void isoEngineGetMouseTilePos(isoEngineT *isoEngine, point2DT *mouseTilePos)
{
    if(isoEngine == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine is NULL!","error.txt");
        return;
    }
    if(isoEngine->isoMap == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine->isoMap is NULL!","error.txt");
        return;
    }

    point2DT point;
    point2DT tileShift, mouse2IsoPOint;

    if(isoEngine == NULL || mouseTilePos == NULL){
        return;
    }

    int modulusX = isoEngine->isoMap->tileSize*isoEngine->zoomLevel;
    int modulusY = isoEngine->isoMap->tileSize*isoEngine->zoomLevel;
    int correctX =(((int)isoEngine->mapScroll2Dpos.x)%modulusX)*2;
    int correctY = ((int)isoEngine->mapScroll2Dpos.y)%modulusY;

    //copy mouse point
    mouse2IsoPOint = isoEngine->mousePoint;
    isoEngineConvertIsoTo2D(&mouse2IsoPOint);

    //get tile coordinates
    isoEngineGetTileCoordinates(isoEngine,&mouse2IsoPOint,&point);

    tileShift.x = correctX;
    tileShift.y = correctY;
    isoEngineConvert2dToIso(&tileShift);

    //check for fixing tile position when the y position is larger than 0
    if(isoEngine->mapScroll2Dpos.y>0){
        point.y -= (((float)isoEngine->scrollY-tileShift.y)/(float)isoEngine->isoMap->tileSize)/isoEngine->zoomLevel;
        point.y+=1;
    }
    else{
        point.y -= (((float)isoEngine->scrollY-tileShift.y)/(float)isoEngine->isoMap->tileSize)/isoEngine->zoomLevel;
    }

    //check for fixing tile position when the x position is larger than 0
    if(isoEngine->mapScroll2Dpos.x>0)
    {
        point.x -= (((float)isoEngine->scrollX+(float)tileShift.x)/(float)isoEngine->isoMap->tileSize)/isoEngine->zoomLevel;
        point.x+=1;
    }
    else{
        point.x -= (((float)isoEngine->scrollX+(float)tileShift.x)/(float)isoEngine->isoMap->tileSize)/isoEngine->zoomLevel;
    }
    mouseTilePos->x = (int)point.x;
    mouseTilePos->y = (int)point.y;
}

void isoEngineCenterMapToTileUnderMouse(isoEngineT *isoEngine)
{
    if(isoEngine == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine is NULL!","error.txt");
        return;
    }
    if(isoEngine->isoMap == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine->isoMap is NULL!","error.txt");
        return;
    }
    point2DT mouseIsoTilePos;

    //calculate the offset of the center of the screen
    int offsetX = WINDOW_WIDTH/isoEngine->zoomLevel/2;
    int offsetY = WINDOW_HEIGHT/isoEngine->zoomLevel/2;

    //get the tile under the mouse
    isoEngineGetMouseTilePos(isoEngine,&mouseIsoTilePos);

    isoEngine->tilePos.x = mouseIsoTilePos.x*isoEngine->isoMap->tileSize;
    isoEngine->tilePos.y = mouseIsoTilePos.y*isoEngine->isoMap->tileSize;

    //convert to isometric coordinates
    isoEngineConvert2dToIso(&mouseIsoTilePos);

    //move the x position
    isoEngine->mapScroll2Dpos.x = ((mouseIsoTilePos.x*isoEngine->isoMap->tileSize)*isoEngine->zoomLevel)/2;
    isoEngine->mapScroll2Dpos.x -= (offsetX*isoEngine->zoomLevel)/2;

    //move the y position
    isoEngine->mapScroll2Dpos.y = -((mouseIsoTilePos.y*isoEngine->isoMap->tileSize)*isoEngine->zoomLevel);
    isoEngine->mapScroll2Dpos.y += offsetY*isoEngine->zoomLevel;

    //convert the map 2d camera to isometric camera
    isoEngineConvertCartesianCameraToIsometric(isoEngine,&isoEngine->mapScroll2Dpos);
}

void isoEngineCenterMap(isoEngineT *isoEngine,point2DT *objectPoint)
{
    point2DT pointPos = *objectPoint;

    //calculate the offset of the center of the screen
    int offsetX = WINDOW_WIDTH/isoEngine->zoomLevel/2;
    int offsetY = WINDOW_HEIGHT/isoEngine->zoomLevel/2;

    isoEngine->tilePos.x = objectPoint->x;
    isoEngine->tilePos.y = objectPoint->y;

    isoEngineConvert2dToIso(&pointPos);

    isoEngine->mapScroll2Dpos.x = floor((pointPos.x)*isoEngine->zoomLevel)/2;
    isoEngine->mapScroll2Dpos.x -= offsetX*isoEngine->zoomLevel/2;

    /* - Hardcoded, change this later in when Entity Component System is completed
    if(game.gameMode == GAME_MODE_OBJECT_FOCUS){
        isoEngine->mapScroll2Dpos.x +=45*isoEngine->zoomLevel/2;
    }*/

    isoEngine->mapScroll2Dpos.y = -floor((pointPos.y)*isoEngine->zoomLevel);
    isoEngine->mapScroll2Dpos.y += offsetY*isoEngine->zoomLevel;

    /* - Hardcoded, change this later in when Entity Component System is completed
    if(game.gameMode == GAME_MODE_OBJECT_FOCUS){
        isoEngine->mapScroll2Dpos.y -= 51*isoEngine->zoomLevel;
    }*/

    isoEngineConvertCartesianCameraToIsometric(isoEngine,&isoEngine->mapScroll2Dpos);
}


void isoEngineGetMouseTileClick(isoEngineT *isoEngine)
{
    if(isoEngine == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine is NULL!","error.txt");
        return;
    }
    if(isoEngine->isoMap == NULL){
        writeToLog("Error in function isoEngineDrawIsoMouse(...) - isoEngine->isoMap is NULL!","error.txt");
        return;
    }
    point2DT point;
    isoEngineGetMouseTilePos(isoEngine,&point);
    if(point.x>=0 && point.y>=0 && point.x<isoEngine->isoMap->mapWidth && point.y<isoEngine->isoMap->mapHeight)
    {
        isoEngine->lastTileClicked = isoMapGetTile(isoEngine->isoMap,(int)point.x,(int)point.y,0);
    }
}
