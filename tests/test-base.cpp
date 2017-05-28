#define CATCH_CONFIG_CPP11_OR_GREATER
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WIN32

#define GLEXTL_IMPLEMENTATION
#include <GL/glextl.h>

#endif // _WIN32
