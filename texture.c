#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include "IsoEngine/isoEngine.h"
#include "renderer.h"
#include "texture.h"
#include "logger.h"

int loadTexture(textureT *texture, char *filename)
{
    char msg[200];
    SDL_Surface *tmpSurface = IMG_Load(filename);

    if(tmpSurface == NULL){
        sprintf(msg,"Texture error: Could not load image:%s! SDL_image Error:%s\n",filename,IMG_GetError());
        writeToLog(msg,"error.log");
        return 0;
    }
    else{
        texture->texture = SDL_CreateTextureFromSurface(getRenderer(),tmpSurface);

        if(texture->texture == NULL){
            sprintf(msg,"Texture error: Could not load image:%s! SDL_image Error:%s\n",filename,IMG_GetError());
            writeToLog(msg,"error.log");
            SDL_FreeSurface(tmpSurface);
            return 0;
        }
        else{
            texture->width = tmpSurface->w;
            texture->height = tmpSurface->h;
        }
        SDL_FreeSurface(tmpSurface);
        return 1;
    }
    return 0;
}

void textureInit(textureT *texture, int x,int y, double angle, SDL_Point *center, SDL_Rect *cliprect, SDL_RendererFlip fliptype)
{
    texture->x = x;
    texture->y = y;
    texture->angle = angle;
    texture->center = center;
    texture->fliptype = fliptype;
    texture->cliprect = cliprect;
}

void textureRenderXYClip(textureT *texture, int x, int y, SDL_Rect *cliprect)
{
    char msg[200];
    if(texture==NULL){
        sprintf(msg,"Warning: passed texture was null!\n");
        writeToLog(msg,"error.txt");
        return;
    }
    texture->x = x;
    texture->y = y;
    texture->cliprect = cliprect;
    SDL_Rect quad = { texture->x, texture->y, texture->width, texture->height };
    if(texture->cliprect != NULL)
    {
        quad.w = texture->cliprect->w;
        quad.h = texture->cliprect->h;
    }

    SDL_RenderCopyEx(getRenderer(),texture->texture,texture->cliprect,&quad,texture->angle, texture->center,texture->fliptype);
}
void textureRenderXYClipScale(textureT *texture, int x, int y, SDL_Rect *cliprect,float scale)
{
    float w,h;
    float diffx,diffy;
    SDL_Rect quad;
    w=(float)texture->width+1*scale;
    h=(float)texture->height+1*scale;

    diffx = (x*scale) - x;
    diffy = (y*scale) - y;

    setupRect(&quad,(x*scale)-diffx,(y*scale)-diffy,w,h);

    texture->cliprect = cliprect;

    if(texture->cliprect != NULL){
        quad.w = (int)texture->cliprect->w*scale;
        quad.h = (int)texture->cliprect->h*scale;

        if(scale <1.0 || scale >1.0){
            quad.h +=1;
            quad.w +=1;
        }
    }
    SDL_RenderCopyEx(getRenderer(),texture->texture,texture->cliprect,&quad,texture->angle,texture->center,texture->fliptype);
}

void textureDelete(textureT *texture)
{
    if(texture!=NULL)
    {
        if(texture->texture != NULL){
            SDL_DestroyTexture(texture->texture);
        }
    }
}
