#include <os.h>
#include <stdlib.h>
#include <time.h>
#include "n2DLib.h"
#include "pieces.h"
#include "sprites.h"

#define GRID_W 10
#define GRID_H 23
#define GRID_Y 10
#define GRID_X 105
#define GRID_SPAWN 3

#define TIMER 0x900D0000
unsigned timer_ctl_bkp[2], timer_load_bkp[2];
int bag[7];
int in_bag = 0;

//#define DEBUG


void draw_tilemap(const unsigned map[])
{
	unsigned x, y;
	unsigned tile;

	for (y = GRID_SPAWN; y < GRID_H; y++)
	for (x = 0; x < GRID_W; x++)
	{
		tile = map[y * GRID_W + x];
		drawSprite(color[tile], (y - GRID_SPAWN) * 11 ,240- x * 11 );
	}
}

void piece_draw(unsigned piece, unsigned orientation, unsigned x, unsigned y)
{
	unsigned i, j;
	unsigned tile;

	for (j = 0; j < 4; j++)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0 && (y + j) >= GRID_SPAWN)
			drawSprite(color[tile], (y + j - GRID_SPAWN) * 11 , 240 - (x + i) * 11 );
	}	
}

unsigned piece_merge(unsigned piece, unsigned orientation, unsigned x, unsigned y, unsigned map[])
{
	unsigned i, j;
	unsigned tile;

	for (j = 3; j < 4; j--)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0 && (x + i) < GRID_W && (y + j) < GRID_H)
		{
			map[(x + i) + (y + j) * GRID_W] = tile;
			if ((y + j) < 2)
				return 1;
		}
#ifdef DEBUG
		else if (tile > 0)
		{
			printf("Invalid write to map : %u, %u, %u, %u\n", tile, orientation, x + i, y + j);
		}
#endif
	}
	return 0;
}

unsigned piece_collide(unsigned piece, unsigned orientation, unsigned x, unsigned y, unsigned map[])
{
	unsigned i, j;
	unsigned tile;

	for (j = 0; j < 4; j++)
	for (i = 0; i < 4; i++)
	{
		tile = pieces[piece][orientation][i + j * 4];
		if (tile > 0)
		{
			if (map[(x + i) + (y + j) * GRID_W])
				return 1;

			if ((x + i) >= GRID_W || (y + j) >= GRID_H)
				return 1;
		}
	}
	return 0;
}

unsigned key_left()
{
	return isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_4);
}

unsigned key_right()
{
	return isKeyPressed(KEY_NSPIRE_RIGHT) || isKeyPressed(KEY_NSPIRE_6);
}

unsigned key_up()
{
	return isKeyPressed(KEY_NSPIRE_UP) || isKeyPressed(KEY_NSPIRE_8);
}

unsigned key_down()
{
	return isKeyPressed(KEY_NSPIRE_DOWN) || isKeyPressed(KEY_NSPIRE_5) || isKeyPressed(KEY_NSPIRE_2);
}

//bag algorithm
//I have no idea whatI'm doing and I'll probably break everything
int bag_piece()
{
	int i;
	if ( in_bag > 0 )
	{
		in_bag--;
		i = bag[in_bag];
		bag[in_bag] = -1;
		return i;
	}
	while( in_bag < 7 )
	{
		i = rand()%7;
		int z = 0;
		int j;
		for( j = 0; j < 7; j++)
		{
			if( i == bag[j] ) z++;
		}
		if( z == 0 )
		{
			bag[in_bag] = i;
			in_bag++;
		}
		
	}
	bag[--in_bag] = -1;
	return i;
}
	


int main(void)
{
	timer_init(0);
	timer_init(1);

	srand(time(NULL)); // RNG seed
	unsigned i, j; // Loop index

	unsigned x = 3, y = 0;
	//int txtx = 0, txty = 0;// text displaying ints
	unsigned cur_piece = rand() % 7, rot = 0;
	unsigned speed = 16384, key_delay = 8192;

	unsigned map[GRID_H * GRID_W];
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
			timer_load(0, 0);
		}

		piece_draw(cur_piece, rot, x, y);
		//txtx = 10;
		//txty = 10;
		//drawDecimal(&txtx, &txty, 240 - ( (x ) * 11) + GRID_Y,0,65335);
		updateScreen();

		if (timer_read(1) == 0)
		{
			if (! piece_collide(cur_piece, rot, x, y + 1, map))
			{
				y++;
			}
			else
			{
				if (piece_merge(cur_piece, rot, x, y, map)) // THE GAME
				{
#ifdef DEBUG
					printf("THE GAME\n");
#endif
					deinitBuffering();
					timer_restore(0);
					timer_restore(1);
					return 0;
				}
				x = 3; y = 0;
				rot = 0;
				cur_piece = bag_piece();
#ifdef DEBUG
				printf("Spawn : %u, %u\n", cur_piece, rot);
#endif

				for (i = GRID_H - 1; i < GRID_H; i--)
				{
					unsigned k = 0;
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

