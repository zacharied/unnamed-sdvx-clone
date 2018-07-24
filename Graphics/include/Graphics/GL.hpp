/*
	OpenGL include file
	This file includes the appropriate opengl headers for the platform
*/
#pragma once

// Link statically against glew
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

// We don't need GLU
#ifndef GLEW_NO_GLU
#define GLEW_NO_GLU
#endif

// Include platform specific OpenGL headers
#ifdef _WIN32
#include <Graphics/glew/GL/glew.h>
#include <Graphics/glew/GL/wglew.h>
#elif __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#elif __linux
#include "GLES3/gl3.h"
#include "GLES3/gl3ext.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

