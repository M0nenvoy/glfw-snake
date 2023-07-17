#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERNAL
#include "render.h"

GLuint render_sp(const char *source_vertex, const char *source_fragment)
{
	int ok;
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &source_vertex, NULL);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char info[512];
		glGetShaderInfoLog(vshader, sizeof(info), NULL, info);
		fprintf(stderr, "Failed compiling a vertex shader:\n%s", info);
		return 0;
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &source_fragment, NULL);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char info[512];
		glGetShaderInfoLog(fshader, sizeof(info), NULL, info);
		fprintf(stderr, "Failed compiling a fragment shader:\n%s", info);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	glDeleteShader(fshader);
	glDeleteShader(vshader);
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (!ok) {
		char info[512];
		glGetProgramInfoLog(program, sizeof(info), NULL, info);
		fprintf(stderr, "Failed linking a shader program:\n%s", info);
		return 0;
	}

	return program;
}

GLuint render_vao_default(void)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	glBindVertexArray(0);
	glBindBuffer(GL_VERTEX_ARRAY, 0);

	return vao;
}

RenderContext *internal_ctx_allocate(int capacity, int vertices_per_instance)
{
	RenderContext *retval = calloc(1, sizeof(*retval) + capacity * vertices_per_instance * sizeof(*retval->vertices));
	retval->capacity = capacity;
	retval->vertices_per_instance = vertices_per_instance;
	retval->dirty = 0;
	return retval;
}

RenderContext *render_ctx_line(int capacity)
{
	RenderContext *ctx = internal_ctx_allocate(capacity, 4);
	ctx->primitive = GL_LINES;

	glGenVertexArrays(1, &ctx->vao);
	glBindVertexArray(ctx->vao);

	glGenBuffers(1, &ctx->buffers[0]);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, capacity * ctx->vertices_per_instance * sizeof(*ctx->vertices), ctx->vertices, GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(*ctx->vertices), 0);

	assert(glGetError() == GL_NO_ERROR);

	return ctx;
}

RenderContext *render_ctx_square(int capacity)
{
	RenderContext *ctx = internal_ctx_allocate(capacity, 8);
	ctx->indices_per_instance = 6;
	ctx->primitive = GL_TRIANGLES;

	glGenVertexArrays(1, &ctx->vao);
	glBindVertexArray(ctx->vao);

	glGenBuffers(2, ctx->buffers);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->buffers[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->buffers[1]);

	unsigned int *indices = malloc(6 * capacity * sizeof(*indices));
	int offs = 0;
	for (int i = 0; i < 6 * capacity; offs += 4) {
		indices[i++] = 0 + offs;
		indices[i++] = 1 + offs;
		indices[i++] = 2 + offs;
		indices[i++] = 3 + offs;
		indices[i++] = 2 + offs;
		indices[i++] = 0 + offs;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * capacity * sizeof(*indices), indices, GL_STATIC_DRAW);
	free(indices);
	glBufferData(GL_ARRAY_BUFFER, capacity * ctx->vertices_per_instance * sizeof(*ctx->vertices), ctx->vertices, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(*ctx->vertices), 0);

	assert(glGetError() == GL_NO_ERROR);

	return ctx;
}

void render_ctx_write(RenderContext *ctx, int position, int n, const float *vertices)
{
	assert(ctx->capacity >= position + n);
	ctx->dirty = 1;
	memcpy(&ctx->vertices[position * ctx->vertices_per_instance], vertices, n * ctx->vertices_per_instance * sizeof(*ctx->vertices));
}

void render_ctx_update(RenderContext *ctx)
{
	if (!ctx->dirty) {
		return;
	}
	ctx->dirty = 0;
	glBindBuffer(GL_ARRAY_BUFFER, ctx->buffers[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ctx->capacity * ctx->vertices_per_instance * sizeof(*ctx->vertices), ctx->vertices);

	GLuint error = glGetError();
	assert(error == GL_NO_ERROR);
}

void render_ctx_draw(RenderContext *ctx)
{
	glBindVertexArray(ctx->vao);
	if (ctx->indices_per_instance) {
		glDrawElements(ctx->primitive, ctx->capacity * ctx->indices_per_instance, GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(ctx->primitive, 0, ctx->capacity * ctx->vertices_per_instance);
	}
	glBindVertexArray(0);
}

void render_ctx_clear(RenderContext *ctx)
{
	memset(ctx->vertices, 0, ctx->capacity * ctx->vertices_per_instance * sizeof(*ctx->vertices));
}

void render_ctx_destroy(RenderContext *ctx)
{
	free(ctx);
}
