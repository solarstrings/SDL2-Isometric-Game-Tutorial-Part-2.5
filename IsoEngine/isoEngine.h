#ifndef ISOENGINE_H_
#define ISOENGINE_H_
#include <SDL2/SDL.h>
#include "isoMap.h"

typedef struct point2DT
{
    float x;
    float y;
}point2DT;

typedef struct isoEngineT
{
    int scrollX;
    int scrollY;
    int mapScrollSpeed;
    point2DT mapScroll2Dpos;
    float zoomLevel;
    SDL_Rect mouseRect;
    point2DT mousePoint;
    point2DT tilePos;
    int lastTileClicked;
    isoMapT *isoMap;
}isoEngineT;

void setupRect(SDL_Rect *rect,int x,int y,int w,int h);
isoEngineT *isoEngineNewIsoEngine();
void isoEngineInit(isoEngineT *isoEngine, int tileSizeInPixels);
void isoEngineFreeIsoEngine(isoEngineT *isoEngine);
void IsoEngineSetMapSize(isoEngineT *isoEngine,int width, int height);
void isoEngineConvert2dToIso(point2DT *point);
void isoEngineConvertIsoTo2D(point2DT *point);
void isoEngineGetTileCoordinates(isoEngineT *isoEngine,point2DT *point,point2DT *point2DCoord);

void isoEngineConvertIsoCameraToCartesian(isoEngineT *isoEngine,point2DT *cartesianCamPos);
void isoEngineConvertCartesianCameraToIsometric(isoEngineT *isoEngine,point2DT *cartesianCamPos);

void isoEngineUpdateMousePos(isoEngineT *isoEngine);
void isoEngineScrollMapWithMouse(isoEngineT *isoEngine);
void isoEngineDrawIsoMouse(isoEngineT *isoEngine);
void isoEngineDrawIsoMap(isoEngineT *isoEngine);
void isoEngineGetMouseTilePos(isoEngineT *isoEngine, point2DT *mouseTilePos);
void isoEngineCenterMapToTileUnderMouse(isoEngineT *isoEngine);
void isoEngineCenterMap(isoEngineT *isoEngine,point2DT *objectPoint);
void isoEngineGetMouseTileClick(isoEngineT *isoEngine);

#endif // ISOENGINE_H_
