#include <os.h>
#include <stdlib.h>
#include <time.h>
#include "n2DLib.h"
#include "pieces.h"
#include "sprites.h"

#define GRID_W 10
#define GRID_H 20
#define GRID_X 10
#define GRID_Y 105


void draw_tilemap(const uint8_t map[])
{
	int x, y;
	uint8_t tile;

	for (y = 0; y < GRID_H; y++)
	for (x = 0; x < GRID_W; x++)
	{
		tile = map[y * GRID_W + x];
		drawSprite(color[tile], x * 11 + GRID_Y, y * 11 + GRID_X);
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
			drawSprite(color[tile], (x + i) * 11 + GRID_Y, (y + j) * 11 + GRID_X);
	}
}

void piece_merge(int piece, int orientation, int x, int y, uint8_t map[])
{
	int i, j;
	uint8_t tile;

	for (j = 3; j >= 0; j--)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0)
			map[(x + i) + (y + j) * GRID_W] = tile;
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
			if (map[(x + i) + (y + j) * GRID_W])
				return 1;

			if ((x + i) < 0 || (x + i) >= GRID_W || (y + j) >= GRID_H)
				return 1;
		}
	}
	return 0;
}

int key_left()
{
	return isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_4);
}

int key_right()
{
	return isKeyPressed(KEY_NSPIRE_RIGHT) || isKeyPressed(KEY_NSPIRE_6);
}

int key_up()
{
	return isKeyPressed(KEY_NSPIRE_UP) || isKeyPressed(KEY_NSPIRE_8);
}

int key_down()
{
	return isKeyPressed(KEY_NSPIRE_DOWN) || isKeyPressed(KEY_NSPIRE_5) || isKeyPressed(KEY_NSPIRE_2);
}

int main(void)
{
	srand(time(NULL)); // RNG seed
	int i, j; // Loop index

	int x = 4, y = 0;
	int cur_piece = 0, rot = 0;

	uint8_t map[GRID_H * GRID_W];
	for (i = 0; i < GRID_H * GRID_W; i++) map[i] = 0; // Map init

	initBuffering();
	clearBufferW();

	while (! isKeyPressed(KEY_NSPIRE_ESC))
	{
		for (i = 0; i < 8; i++)
		{
			draw_tilemap(map);
			if(key_up() && ! piece_collide(cur_piece, (rot + 1) % 4, x, y, map))
				rot = (rot + 1) % 4;

			if (key_right() && ! piece_collide(cur_piece, rot, x + 1, y, map))
				x++;

			if (key_left() && ! piece_collide(cur_piece, rot, x - 1, y, map))
				x--;

			if (key_down() && ! piece_collide(cur_piece, rot, x, y + 1, map))
				y++;

			if (isKeyPressed(KEY_NSPIRE_PLUS))
				cur_piece = (cur_piece + 1) % 7;

			piece_draw(cur_piece, rot, x, y);
			updateScreen();
			sleep(50);
		}

		if (! piece_collide(cur_piece, rot, x, y + 1, map))
		{
			y++;
		}
		else
		{
			piece_merge(cur_piece, rot, x, y, map);
			y = 0;
			x = 4;
			rot = 0;
			cur_piece = rand() % 7;
		}

		for (i = GRID_H - 1; i >= 0; i--)
		{
			int k = 0;
			for (j = 0; j < GRID_W; j++)
			{
				if (map[j + i * GRID_W] > 0)
					k++;
			}
			if (k == GRID_W)
			{
				for (j = 0; j < GRID_W; j++)
					map[j + i * GRID_W] = 0;

				for (j = i; j > 0; j--)
				for (k = 0; k < GRID_W; k++)
					map[k + j * GRID_W] = map[k + (j - 1) * GRID_W];

				for (j = 0; j < GRID_W; j++)
					map[j] = 0;

				i++;
			}
		}
	}

	deinitBuffering();
	return 0;
}

