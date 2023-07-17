all:
	gcc snake.c game.c render.c collections.c glad.c -o snake -lglfw -lGL -lm -lpthread -Wall -Wextra -g -O0 -Iinclude
