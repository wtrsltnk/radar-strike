#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
#define SHADER_PATH "shaders/gl3/"
#else
#define SHADER_PATH "shaders/gles3/"
#endif // _WIN32

#endif // PLATFORM_H
