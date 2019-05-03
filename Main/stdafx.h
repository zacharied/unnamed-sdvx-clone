/* Main and precompiled header file for Main project*/
#pragma once

// OpenGL headers
#include <Graphics/GL.hpp>

#ifdef _WIN32
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#endif

// C RunTime Header Files
#include <stdlib.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <memory.h>
#include <cinttypes>

// TODO: reference additional headers your program requires here
#include <Shared/Shared.hpp>

// Graphics components
#include <Graphics/Graphics.hpp>
using namespace Graphics;

// Asset loading macro
#define CheckedLoad(__stmt) if(!(__stmt)){Logf("Failed to load asset [%s]", Logger::Error, #__stmt); return false; }