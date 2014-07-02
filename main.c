#include <os.h>
#include <stdlib.h>
#include <time.h>
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
		drawSprite(color[tile], x * 11 + 105, y * 11 + 10);
	}
}

void piece_draw(int piece, int orientation, int x, int y)
{
	int i, j;
	uint8_t tile;
	for (j = 0; j < 4; j++)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0)
			drawSprite(color[tile], (x + i) * 11 + 105, (y + j) * 11 + 10);
	}
}

void piece_merge(int piece, int orientation, int x, int y, uint8_t map[])
{
	int i, j;
	uint8_t tile;
	for (j = 0; j < 4; j++)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0)
			map[(x + i) + (y + j) * 10] = tile;
	}
}

int piece_collide(int piece, int orientation, int x, int y, uint8_t map[])
{
	int i, j;
	uint8_t tile;
	for (j = 0; j < 4; j++)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0)
		{
			if (map[(x + i) + (y + j) * 10])
				return 1;

			if ((x + i) < 0 || (x + i) >= 10 || (y + j) >= 20)
				return 1;
		}
	}
	return 0;
}

int main(void)
{
	srand(time(NULL));
	int i;
	uint8_t map[200];
	for (i = 0; i < 200; i++) map[i] = 0;
	piece_merge(4, 7, 0, 0, map);
	if (! piece_collide(4, 7, 2, 0, map)) piece_merge(4, 7, 2, 0, map);

	int block_x,block_y, rot, score;
	initBuffering();
	clearBufferW();

	while (! isKeyPressed(KEY_NSPIRE_ESC))
	{
		draw_tilemap(map);
		piece_draw(rand() % 7, rand() % 4, 3, 10);
		updateScreen();
	}
	deinitBuffering();
	return 0;
}

