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

#define TIMER 0x900D0000
unsigned timer_ctl_bkp[2], timer_load_bkp[2];

//#define DEBUG

void timer_init(int timer)
{
	if (is_cx)
	{
		volatile unsigned *timer_ctl = (unsigned *) (TIMER + 0x08 + 0x20 * timer);
		volatile unsigned *timer_load = (unsigned *) (TIMER + 0x20 * timer);

		timer_ctl_bkp[timer] = *timer_ctl;
		timer_load_bkp[timer] = *timer_load;

		*timer_ctl &= ~(1 << 7);
		*timer_ctl = 0b01100011;
		*timer_ctl |= (1 << 7);
	}
	else
	{
	}
}

void timer_restore(int timer)
{
	if (is_cx)
	{
		volatile unsigned *timer_ctl = (unsigned *) (TIMER + 0x08 + 0x20 * timer);
		volatile unsigned *timer_load = (unsigned *) (TIMER + 0x20 * timer);

		*timer_ctl &= ~(1 << 7);
		*timer_ctl = timer_ctl_bkp[timer] & ~(1 << 7);
		*timer_load = timer_load_bkp[timer];
		*timer_ctl = timer_ctl_bkp[timer];
	}
	else
	{
	}
}

void timer_load(int timer, unsigned value)
{
	if (is_cx)
	{
		volatile unsigned *timer_load = (unsigned *) (TIMER + 0x20 * timer);
		*timer_load = value;
	}
	else
	{
	}
}

unsigned timer_read(int timer)
{
	if (is_cx)
	{
		volatile unsigned *timer_value = (unsigned *) (TIMER + 0x04 + 0x20 * timer);
		return *timer_value;
	}
	else
	{
		return 0; // Dummy
	}
}

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
		if (tile > 0 && (x + i) >= 0)
			drawSprite(color[tile], (x + i) * 11 + GRID_Y, (y + j) * 11 + GRID_X);
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
		if (tile > 0 && (x + i) >= 0 && (y + j) >= 0 && (x + i) < GRID_W && (y + j) < GRID_H)
		{
#ifdef DEBUG
			printf("Merging : %u, %u\n", tile, (x + i) + (y + j) * GRID_W);
#endif
			map[(x + i) + (y + j) * GRID_W] = tile;
		}
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
	timer_init(0);
	timer_init(1);

	srand(time(NULL)); // RNG seed
	int i, j; // Loop index

	int x = 3, y = -2;
	int cur_piece = rand() % 7, rot = 0;
	unsigned speed = 16384, key_delay = 8192;

	uint8_t map[GRID_H * GRID_W];
	for (i = 0; i < GRID_H * GRID_W; i++) map[i] = 0; // Map init

	initBuffering();
	clearBufferW();

	timer_load(1, speed);

	while (! isKeyPressed(KEY_NSPIRE_ESC))
	{
		draw_tilemap(map);

		if (key_up() || key_right() || key_left() || key_down() || isKeyPressed(KEY_NSPIRE_PLUS))
		{
			if (timer_read(0) == 0)
			{
				if (key_up() && ! piece_collide(cur_piece, (rot + 1) % 4, x, y, map))
					rot = (rot + 1) % 4;

				if (key_right() && ! piece_collide(cur_piece, rot, x + 1, y, map))
					x++;

				if (key_left() && ! piece_collide(cur_piece, rot, x - 1, y, map))
					x--;

				if (key_down() && ! piece_collide(cur_piece, rot, x, y + 1, map))
				{
					timer_load(1, speed);
					y++;
				}

				if (isKeyPressed(KEY_NSPIRE_PLUS))
					cur_piece = (cur_piece + 1) % 7;

				timer_load(0, key_delay);
				key_delay = 1024;
			}
		}
		else
		{
			key_delay = 8192;
		}

		piece_draw(cur_piece, rot, x, y);
		updateScreen();

		if (timer_read(1) == 0)
		{
			if (! piece_collide(cur_piece, rot, x, y + 1, map))
			{
				y++;
			}
			else
			{
				piece_merge(cur_piece, rot, x, y, map);
				x = 3; y = -2;
				rot = 0;
				cur_piece = rand() % 7;

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
			timer_load(1, speed);
		}
	}

	deinitBuffering();
	timer_restore(0);
	timer_restore(1);
	return 0;
}

