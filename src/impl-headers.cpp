
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WIN32

#define GLEXTL_IMPLEMENTATION
#include <GL/glextl.h>

#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#else

#include <GLES/gl.h>
#include <GLES3/gl3.h>

#include "nanovg.h"
#define NANOVG_GLES3_IMPLEMENTATION
#include "nanovg_gl.h"

#endif // _WIN32
