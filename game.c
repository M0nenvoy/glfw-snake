#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"

#define GAME_INTERNAL
#include "game.h"


int internal_is_inbounds(GameContext *game, int x, int y)
{
	return x < game->width && x >= 0 &&
		y < game->height && y >= 0;
}

void internal_error_not_started(int line)
{
	fprintf(stderr, "%d: Attempting to interact with a game that hasn't started yet.\n", line);
	exit(-1);
}

void internal_error_already_started(int line)
{
	fprintf(stderr, "%d: Attempting to start an already started game.\n", line);
	exit(-1);
}

void internal_error_position_invalid(int line, int x, int y)
{
	fprintf(stderr, "%d: Attempting to start a game with an invalid position: (%d, %d).\n", line, x, y);
	exit(-1);
}

void internal_is_inside_snake_foreach(void *arg, void *ctx)
{
	int x = ((int *)ctx)[0];
	int y = ((int *)ctx)[1];
	int snake_x = ((int *)arg)[0];
	int snake_y = ((int *)arg)[1];
	((int *)ctx)[2] = ((int *)ctx)[2] || (snake_x == x && snake_y == y);
}

int internal_is_inside_snake(GameContext *game, int x, int y)
{
	int context[3] = { x, y, 0 };
	collections_queue_callback_context_set(game->positions_queue, context);
	collections_queue_foreach(game->positions_queue, internal_is_inside_snake_foreach);

	return context[2];
}

void internal_respawn_food(GameContext *game)
{
	game->food_x = rand() % (game->width - 1);
	game->food_y = rand() % (game->height - 1);

	int nx = game->snake_x + game->move_x;
	int ny = game->snake_y + game->move_y;

	if ((nx == game->food_x && ny == game->food_y) || internal_is_inside_snake(game, game->food_x, game->food_y)) {
		internal_respawn_food(game);
	}
}

GameContext *game_create(int width, int height)
{
	GameContext *game = calloc(1, sizeof(*game));
	game->width = width;
	game->height = height;
	game->positions_queue = collections_queue_create(width * height, 2 * sizeof(int));
	return game;
}

void game_start(GameContext *game, int snake_x, int snake_y)
{
	if (game->started) internal_error_already_started(__LINE__);
	if (!internal_is_inbounds(game, snake_x, snake_y)) internal_error_position_invalid(__LINE__, snake_x, snake_y);

	game->snake_x = snake_x;
	game->snake_y = snake_y;
	game->move_x = 0;
	game->move_y = 0;
	game->snake_length = 2;
	game->started = 1;
	internal_respawn_food(game);

	collections_queue_empty(game->positions_queue);

	int xy[] = { snake_x, snake_y };
	collections_queue_add(game->positions_queue, xy);
}

int game_update(GameContext *game)
{
	if (!game->started) internal_error_not_started(__LINE__);
	if (game->move_x + game->move_y == 0) return 0;

	int new_x = game->snake_x + game->move_x;
	int new_y = game->snake_y + game->move_y;

	if (!internal_is_inbounds(game, new_x, new_y)) {
		game->started = 0;
		return 1;
	}

	if (internal_is_inside_snake(game, new_x, new_y)) {
		game->started = 0;
		return 1;
	}

	if (new_x == game->food_x && new_y == game->food_y)
	{
		game->snake_length++;
		internal_respawn_food(game);
	}

	if (collections_queue_size(game->positions_queue) >= game->snake_length) {
		collections_queue_pop_first(game->positions_queue);
	}

	game->snake_x = new_x;
	game->snake_y = new_y;
	int positions[] = { new_x, new_y };
	collections_queue_add(game->positions_queue, &positions);

	return 0;
}

void game_get_food(GameContext *game, int *position)
{
	position[0] = game->food_x;
	position[1] = game->food_y;
}

void game_callback_context_set(GameContext *game, void *context)
{
	collections_queue_callback_context_set(game->positions_queue, context);
}

void game_snake_foreach(GameContext *game, void func(void *context, void *arg))
{
	collections_queue_foreach(game->positions_queue, func);
}

int game_set_snake_direction(GameContext *game, GameSnakeDirection direction)
{
	if (!game->started) internal_error_not_started(__LINE__);
	switch (direction) {
		case GSD_UP:
		{
			if (abs(game->move_y) == 1) return 0;
			game->move_y = 1;
			game->move_x = 0;
			break;
		}
		case GSD_DOWN:
		{
			if (abs(game->move_y) == 1) return 0;
			game->move_y = -1;
			game->move_x = 0;
			break;
		}
		case GSD_RIGHT:
		{
			if (abs(game->move_x) == 1) return 0;
			game->move_y = 0;
			game->move_x = 1;
			break;
		}
		case GSD_LEFT:
		{
			if (abs(game->move_x) == 1) return 0;
			game->move_y = 0;
			game->move_x = -1;
			break;
		}
                case GSD_NONE:
			return 0;
                }

        return 1;
}

void game_destroy(GameContext *game)
{
	collections_queue_destroy(game->positions_queue);
	free(game);
}
