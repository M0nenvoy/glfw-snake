#ifndef GAME
#define GAME

#ifdef GAME_INTERNAL

#include "collections.h"

typedef struct GameContext {
	int started;
	int move_x, move_y;
	int width, height; /* Size of the map */
	int snake_x, snake_y; /* Snake head position */
	int food_x, food_y; /* Food position */
	int snake_length;
	Queue *positions_queue;
} GameContext;
#endif

#ifndef GAME_INTERNAL
typedef void GameContext;
#endif // !GAME_INTERNAL

typedef enum GameSnakeDirection {
	GSD_NONE = 0,
	GSD_DOWN,
	GSD_UP,
	GSD_RIGHT,
	GSD_LEFT,
} GameSnakeDirection;

GameContext *game_create(int width, int height);
void         game_start(GameContext *game, int snake_x, int snake_y);
int          game_update(GameContext *game); /* Updates the game. Returns 1 if the game is lost. */
void         game_get_food(GameContext *game, int *position); /* Gets the position of a snake. */
void         game_callback_context_set(GameContext *game, void *context); /* Sets the callback context. */
void         game_snake_foreach(GameContext *game, void func(void *context, void *arg)); /* Does something for each snake tile (renders probably) */
int          game_set_snake_direction(GameContext *game, GameSnakeDirection direction); /* Changes a direction. Returns 1 if direction changed successfully. */
void         game_destroy(GameContext *game);

#endif // Game
