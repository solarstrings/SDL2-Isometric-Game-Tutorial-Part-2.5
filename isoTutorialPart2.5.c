/*
 *   Isometric Game Tutorial Part 2.5
 *   Author: Johan Forsblom
 *   Date: 22th November 2017
 *
 *   This tutorial covers the following:
 *
 *      * Refactoring most of the code - same functionality as before
 *      * Dynamic memory allocation for the isometric engine
 *      * Dynamic memory allocation for tile textures
 *      * Dynamic memory allocation for the map - now including multiple layers
 *      * Get a tile from the map: (x,y,layer)
 *      * Set a tile on the map: (x,y,layer)
 *      * Logging errors/warnings/info to text file
 *
 *      NOTE: The character moving/drawing code is not re-factored in this tutorial.
 *            It will be replaced later when the entity component system (ECS)
 *            is developed later in tutorial part (4? or 5?).
 *
 *   Usage:
 *   Space bar -  toggle between Overview mode / Object focus mode
 *   Move the character with w,a,s,d
 *   Zoom in and out with the mouse wheel
 *
 *   Overview mode:
 *   Left click - center map to tile under mouse
 *   Scroll map with the mouse close to the edges
 *
 *   Object focus mode:
 *   Left click on the map for "tile picking" (shows the selected tile up in the top left corner of the screen)
 *
 ******************************************************************************************************************
 *
 *   Copyright 2017 Johan Forsblom
 *
 *   You may use the code for whatever reason you want. If you do a commercial project and took inspiration from, or
 *   copied this code, all i require is a place in the credits section of your game :)
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 *   LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "initclose.h"
#include "renderer.h"
#include "texture.h"
#include "IsoEngine/isoEngine.h"
#include "logger.h"

#define PLAYER_DIR_UP_LEFT      0
#define PLAYER_DIR_UP           1
#define PLAYER_DIR_UP_RIGHT     2
#define PLAYER_DIR_RIGHT        3
#define PLAYER_DIR_DOWN_RIGHT   4
#define PLAYER_DIR_DOWN         5
#define PLAYER_DIR_DOWN_LEFT    6
#define PLAYER_DIR_LEFT         7

#define NUM_ISOMETRIC_TILES 5
#define NUM_CHARACTER_SPRITES 8
#define MAP_HEIGHT 64
#define MAP_WIDTH 64

#define GAME_MODE_OVERVIEW          0
#define GAME_MODE_OBJECT_FOCUS      1
#define NUM_GAME_MODES              2

typedef struct gameT
{
    SDL_Event event;
    int loopDone;
    isoEngineT *isoEngine;
    point2DT charPoint;
    int charDirection;
    int gameMode;
}gameT;

gameT game;
textureT characterTex;
SDL_Rect charRects[NUM_CHARACTER_SPRITES];

void initCharClip()
{
    int x=0,y=0;
    int i;
    textureInit(&characterTex,0,0,0,NULL,NULL,SDL_FLIP_NONE);
    for(i=0;i<NUM_CHARACTER_SPRITES;++i)
    {
        setupRect(&charRects[i],x,y,70,102);
        x+=70;
    }
}

void init()
{
    game.loopDone = 0;
    game.isoEngine = isoEngineNewIsoEngine();
    if(game.isoEngine == NULL){
        closeDownSDL();
        exit(1);
    }
    game.isoEngine->isoMap = isoMapCreateEmptyMap("Testmap",MAP_WIDTH,MAP_HEIGHT,2,64);
    if(game.isoEngine->isoMap == NULL){
        isoEngineFreeIsoEngine(game.isoEngine);
        closeDownSDL();
        exit(1);
    }
    isoMapLoadTileSet(game.isoEngine->isoMap,"data/isotiles.png",64,80);

    setLoggerDirectory("logs");
    initCharClip();
    game.charPoint.x = 0;
    game.charPoint.y = 0;
    game.charDirection = PLAYER_DIR_DOWN;
    game.gameMode = GAME_MODE_OVERVIEW;

    if(loadTexture(&characterTex,"data/character.png")==0){
        writeToLog("Error, could not load texture: data/character.png","error.txt");
        exit(1);
    }
}
void drawCharacter(isoEngineT *isoEngine)
{
    point2DT point;
    point.x = (int)(game.charPoint.x*isoEngine->zoomLevel)+ isoEngine->scrollX;
    point.y = (int)(game.charPoint.y*isoEngine->zoomLevel)+ isoEngine->scrollY;
    isoEngineConvert2dToIso(&point);
    textureRenderXYClipScale(&characterTex,point.x,point.y,&charRects[game.charDirection],isoEngine->zoomLevel);
}

void drawLastTileClicked(isoEngineT *isoEngine)
{
    if(isoEngine->lastTileClicked!=-1){
        textureRenderXYClip(isoEngine->isoMap->tileSet->tilesTex,0,0,&isoEngine->isoMap->tileSet->tileClipRects[isoEngine->lastTileClicked]);
    }
}

void draw()
{
    SDL_SetRenderDrawColor(getRenderer(),0x3b,0x3b,0x3b,0x00);
    SDL_RenderClear(getRenderer());

    isoEngineDrawIsoMap(game.isoEngine);
    drawCharacter(game.isoEngine);
    isoEngineDrawIsoMouse(game.isoEngine);
    drawLastTileClicked(game.isoEngine);

    SDL_RenderPresent(getRenderer());
    //Don't be a CPU HOG!! :D
    SDL_Delay(10);
}

void update(isoEngineT *isoEngine)
{
    isoEngineUpdateMousePos(isoEngine);
    if(game.gameMode == GAME_MODE_OBJECT_FOCUS)
    {
        isoEngineCenterMap(game.isoEngine,&game.charPoint);
    }
    else if(game.gameMode == GAME_MODE_OVERVIEW){
        isoEngineScrollMapWithMouse(game.isoEngine);
    }

}

void updateInput()
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    while(SDL_PollEvent(&game.event) != 0)
    {
        switch(game.event.type)
        {
            case SDL_QUIT:
                game.loopDone=1;
            break;

            case SDL_KEYUP:
                switch(game.event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        game.loopDone=1;
                    break;

                    case SDLK_SPACE:
                        game.gameMode++;
                        if(game.gameMode>=NUM_GAME_MODES)
                        {
                            game.gameMode = GAME_MODE_OVERVIEW;
                        }
                    break;

                    default:break;
                }
            break;

            case SDL_MOUSEBUTTONDOWN:
                if(game.event.button.button == SDL_BUTTON_LEFT)
                {
                    if(game.gameMode==GAME_MODE_OVERVIEW){
                        isoEngineCenterMapToTileUnderMouse(game.isoEngine);

                    }
                    if(game.gameMode == GAME_MODE_OBJECT_FOCUS){
                        isoEngineGetMouseTileClick(game.isoEngine);
                    }
                }
            break;

            case SDL_MOUSEWHEEL:
                //If the user scrolled the mouse wheel up
                if(game.event.wheel.y>=1)
                {
                    if(game.isoEngine->zoomLevel<3.0){
                        game.isoEngine->zoomLevel+=0.25;
                        if(game.gameMode==GAME_MODE_OVERVIEW)
                        {
                            isoEngineCenterMap(game.isoEngine,&game.isoEngine->tilePos);
                        }
                        if(game.gameMode == GAME_MODE_OBJECT_FOCUS){
                            isoEngineCenterMap(game.isoEngine,&game.charPoint);
                        }
                    }
                }
                //If the user scrolled the mouse wheel down
                else{
                    if(game.isoEngine->zoomLevel>1.0){
                        game.isoEngine->zoomLevel-=0.25;
                        if(game.gameMode==GAME_MODE_OVERVIEW)
                        {
                            isoEngineCenterMap(game.isoEngine,&game.isoEngine->tilePos);
                        }
                        if(game.gameMode == GAME_MODE_OBJECT_FOCUS){
                            isoEngineCenterMap(game.isoEngine,&game.charPoint);
                        }
                    }
                }
            break;

            default:break;
        }
    }

    if(keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_W])
    {
        game.charPoint.x+=5;
        game.charPoint.y+=5;
        game.charDirection = PLAYER_DIR_DOWN;
    }
    else if(!keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A] && keystate[SDL_SCANCODE_W])
    {
        game.charPoint.x-=5;
        game.charPoint.y-=5;
        game.charDirection = PLAYER_DIR_UP;
    }
    else if(!keystate[SDL_SCANCODE_S] && keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A] && keystate[SDL_SCANCODE_W])
    {
        game.charPoint.y-=5;
        game.charDirection = PLAYER_DIR_UP_RIGHT;
    }
    else if(!keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_D] && keystate[SDL_SCANCODE_A] && keystate[SDL_SCANCODE_W])
    {
        game.charPoint.x-=5;
        game.charDirection = PLAYER_DIR_UP_LEFT;
    }
    else if(!keystate[SDL_SCANCODE_S] && keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_W])
    {
        game.charPoint.x+=3;
        game.charPoint.y-=3;
        game.charDirection = PLAYER_DIR_RIGHT;
    }
    else if(!keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_D] && keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_W])
    {
        game.charPoint.x-=3;
        game.charPoint.y+=3;
        game.charDirection = PLAYER_DIR_LEFT;
    }
    else if(keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_D] && keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_W])
    {
        game.charPoint.y+=5;
        game.charDirection = PLAYER_DIR_DOWN_LEFT;
    }
    else if(keystate[SDL_SCANCODE_S] && keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_W])
    {
        game.charPoint.x+=5;
        game.charDirection = PLAYER_DIR_DOWN_RIGHT;
    }
/*
    if(keystate[SDL_SCANCODE_W]){

        game.mapScroll2Dpos.y+=game.mapScrolllSpeed;
        isoEngineConvertCartesianCameraToIsometric(game.isoEngine,&game.mapScroll2Dpos);
    }
    if(keystate[SDL_SCANCODE_A]){

        game.mapScroll2Dpos.x-=game.mapScrolllSpeed;
        isoEngineConvertCartesianCameraToIsometric(game.isoEngine,&game.mapScroll2Dpos);
    }
    if(keystate[SDL_SCANCODE_S]){

        game.mapScroll2Dpos.y-=game.mapScrolllSpeed;
        isoEngineConvertCartesianCameraToIsometric(game.isoEngine,&game.mapScroll2Dpos);

    }
    if(keystate[SDL_SCANCODE_D]){

        game.mapScroll2Dpos.x+=game.mapScrolllSpeed;
        isoEngineConvertCartesianCameraToIsometric(game.isoEngine,&game.mapScroll2Dpos);
    }
*/
}

int main(int argc, char *argv[])
{
    initSDL("Isometric Game Tutorial - Part 2.5 - By Johan Forsblom");
    init();

    SDL_ShowCursor(0);
    SDL_SetWindowGrab(getWindow(),SDL_TRUE);
    SDL_WarpMouseInWindow(getWindow(),WINDOW_WIDTH/2,WINDOW_HEIGHT/2);

    while(!game.loopDone){
        update(game.isoEngine);
        updateInput();
        draw();
    }

    closeDownSDL();
    return 0;
}
