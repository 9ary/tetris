#include <os.h>
#include "pieces.h"
#include "sprites.h"
#include "n2DLib.h"

void draw_tilemap(const uint8_t map[])
{
    int x;
    int y;
    uint8_t tile;
    for (y = 0; y < 20; y++)
    for (x = 0; x < 10; x++)
    {
        tile = map[y * 10 + x];
        if(tile > 0)
            drawSprite(cyan, x*11+105, y*11+10);
    }
}

int main(void)
{
    int i;
    uint8_t map[200];
    for (i = 0; i < 200; i++) map[i] = 0;

    int block_x,block_y, rot, score;
    initBuffering();

    while (isKeyPressed(KEY_NSPIRE_ESC) == 0)
    {
       //drawing ops
       clearBufferB(); //need to update this afterward to only erase the tile being drawn and if applicable the top blocks when they drop down; we could use a black tile for  this.

       draw_tilemap(map);
       updateScreen();
    }
    deinitBuffering();
	return 0;
}

