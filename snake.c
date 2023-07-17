#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "collections.h"
#include "render.h"
#include "game.h"

#define INPUT_DOWN  GLFW_KEY_DOWN
#define INPUT_UP    GLFW_KEY_UP
#define INPUT_LEFT  GLFW_KEY_LEFT
#define INPUT_RIGHT GLFW_KEY_RIGHT

#define WINDOW_WIDTH    600
#define WINDOW_HEIGHT   600
#define GRID_SIZE       15
#define UPDATE_INTERVAL 8

typedef struct Cell {
	float vertices[8];
} Cell;

typedef struct {
	int offset;
	RenderContext *render;
} BufferCell_callback;

const float COLOR_BG[3]    = { 0.2f, 0.2f, 0.2f };
const float COLOR_SNAKE[3] = { 0.5f, 0.5f, 0.5f };
const float COLOR_FOOD[3]  = { 0.1f, 0.7f, 0.1f };
const float COLOR_GRID[3]  = { 0.3f, 0.3f, 0.3f };

const char *SOURCE_VERTEX = ""
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 1.0, 1.0);\n"
"}\0";

const char *SOURCE_FRAGMENT = ""
"#version 330 core\n"
"uniform vec3 color;"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(color, 1.0);\n"
"}\0";

/*
 * Makes a cell for a grid of size nxn.
 */
Cell cell_create(int n, int x, int y)
{
	Cell c;
	float inc = 2.0f / n;
	float left = -1.0 + x * inc;
	float right = left + inc;
	float bot = -1.0 + y * inc;
	float top = bot + inc;

	float vertices[8] = {
		left,   bot,
		right,  bot,
		right,  top,
		left,   top,
	};

	memcpy(c.vertices, vertices, sizeof(vertices));

	return c;
}

/*
 * Writes the grid of 'n' horizontal and 'n' vertical lines to the render context.
 */
void grid_write(RenderContext *line_render, int n) {
	assert(n > 0);

	const int mult = 2 * 4;
	float vertices[mult * (n - 1)];
	float inc = 2.0f / n;
	float *vptr = vertices;

	/* Vertical lines */
	for (int i = 1; i < n; ++i) {
		*(vptr++) = -1.0f + i * inc; /* x1 */
		*(vptr++) = -1.0f;           /* y1 */
		*(vptr++) = -1.0f + i * inc; /* x2 */
		*(vptr++) =  1.0f;           /* y2 */
	}

	/* Horizontal lines */
	for (int i = 1; i < n; ++i) {
		*(vptr++) = -1.0f;           /* x1 */
		*(vptr++) = -1.0f + i * inc; /* y1 */
		*(vptr++) =  1.0f;           /* x2 */
		*(vptr++) = -1.0f + i * inc; /* y2 */
	}

	render_ctx_write(line_render, 0, 2 * (n - 1), vertices);
}

/*
 * Processes the input.
 */
void input_process(GLFWwindow *window, GameContext *game, int *update_counter) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
		return;
	}

	GameSnakeDirection direction = 0;

	if (glfwGetKey(window, INPUT_DOWN)) {
		direction = GSD_DOWN;
	}

	if (glfwGetKey(window, INPUT_UP)) {
		direction = GSD_UP;
	}

	if (glfwGetKey(window, INPUT_RIGHT)) {
		direction = GSD_RIGHT;
	}

	if (glfwGetKey(window, INPUT_LEFT)) {
		direction = GSD_LEFT;
	}

	if (!direction) {
		return;
	}

	if (game_set_snake_direction(game, direction) && (*update_counter < (UPDATE_INTERVAL / 3))) {
		*update_counter = 0;
	}
}

void callback_buffer_cell(void *xy, void *callback_BufferCell_instance)
{
	if (callback_BufferCell_instance == NULL) {
		fprintf(stderr, "Callback context is absent.");
		exit(-1);
	}

	BufferCell_callback *instance = (BufferCell_callback*) callback_BufferCell_instance;
	Cell cell = cell_create(GRID_SIZE, ((int *)xy)[0], ((int *)xy)[1]);
	render_ctx_write(instance->render, instance->offset, 1, cell.vertices);
	instance->offset++;
}

void render_loop(GLFWwindow *window, GameContext *game, RenderContext *render_line, RenderContext *render_snake, RenderContext *render_food, GLuint uniform_color)
{
	game_start(game, GRID_SIZE / 2, GRID_SIZE / 2);

	grid_write(render_line, GRID_SIZE);
	render_ctx_update(render_line);

	int frame = 0;
	while (!glfwWindowShouldClose(window)) {
		input_process(window, game, &frame);
		if ((UPDATE_INTERVAL + frame++) % UPDATE_INTERVAL == 0) {
			if (game_update(game)) { /* Restart the game */
				render_ctx_clear(render_snake);
				game_start(game, GRID_SIZE / 2, GRID_SIZE / 2);
			};
			BufferCell_callback fbcc = { 0, render_snake };
			game_callback_context_set(game, &fbcc);
			game_snake_foreach(game, callback_buffer_cell);
			render_ctx_update(render_snake);

			int foodxy[2];
			game_get_food(game, foodxy);
			Cell food = cell_create(GRID_SIZE, foodxy[0], foodxy[1]);
			render_ctx_write(render_food, 0, 1, food.vertices);
			render_ctx_update(render_food);
		}

		glClearColor(COLOR_BG[0], COLOR_BG[1], COLOR_BG[2], 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUniform3f(uniform_color, COLOR_SNAKE[0], COLOR_SNAKE[1], COLOR_SNAKE[2]);
		render_ctx_draw(render_snake);
		glUniform3f(uniform_color, COLOR_FOOD[0], COLOR_FOOD[1], COLOR_FOOD[2]);
		render_ctx_draw(render_food);
		glUniform3f(uniform_color, COLOR_GRID[0], COLOR_GRID[1], COLOR_GRID[2]);
		render_ctx_draw(render_line);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
}

int main(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Learn OpenGL", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize glad. Terminating...");
		glfwTerminate();
		return -2;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	GLuint program = render_sp(SOURCE_VERTEX, SOURCE_FRAGMENT);
	if (!program) {
		return -1;
	}
	glUseProgram(program);

	GameContext   *game         = game_create(GRID_SIZE, GRID_SIZE);
	RenderContext *render_line  = render_ctx_line(2 * GRID_SIZE);
	RenderContext *render_snake = render_ctx_square(GRID_SIZE * GRID_SIZE);
	RenderContext *render_food  = render_ctx_square(1);

	srand(time(NULL));
	render_loop(window, game, render_line, render_snake, render_food, glGetUniformLocation(program, "color"));

	render_ctx_destroy(render_snake);
	render_ctx_destroy(render_food);
	render_ctx_destroy(render_line);
	game_destroy(game);

	glfwTerminate();
	return 0;
}
