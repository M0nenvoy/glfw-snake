#ifndef RENDER
#define RENDER

#include <glad/glad.h>

#ifdef INTERNAL
typedef struct RenderContext {
	GLuint vao;
	int dirty;         /* If needs to be synced with gpu data. */
	GLuint buffers[2]; /* vbo and ebo */
	GLuint primitive;
	int vertices_per_instance;
	int indices_per_instance; /* 0 if doesn't use. */
	int count;                /* Number of instances to draw. */
	int capacity;             /* Capacity of instances. */
	float vertices[];
} RenderContext;

#endif

#ifndef INTERNAL
typedef void RenderContext;
#endif

/** Creates and returns the shader program handle or 0 on failure. */
GLuint render_sp(const char *vertex, const char *fragment);

/** Creates and returns a default vao. */
GLuint render_vao_default(void);

/** Draws triangles. */
void render_triangle_draw(void);

/** Render context for drawing lines. */
RenderContext *render_ctx_line(int capacity);

/** Render context for drawing squares. */
RenderContext *render_ctx_square(int capacity);

/** Write to render context buffer. */
void render_ctx_write(RenderContext *ctx, int position, int n, const float *vertices);

/** Updates the render context (Uploads vertecies to the gpu). */
void render_ctx_update(RenderContext *ctx);

/** Draw using a render context. */
void render_ctx_draw(RenderContext *ctx);

/** Clears the render context. */
void render_ctx_clear(RenderContext *ctx);

/** Destroys render context. */
void render_ctx_destroy(RenderContext *ctx);

#endif // !RENDER
