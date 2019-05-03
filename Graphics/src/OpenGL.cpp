#include "stdafx.h"
#include "OpenGL.hpp"
#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#endif

namespace Graphics
{
	OpenGL::~OpenGL()
	{
		if(context)
		{
			if(glBindProgramPipeline)
				glDeleteProgramPipelines(1, &m_mainProgramPipeline);

			SDL_GL_DeleteContext(context);
			context = nullptr;
		}
	}

	uint32 shaderStageMap[] = {
	GL_VERTEX_SHADER_BIT,
	GL_FRAGMENT_SHADER_BIT,
	GL_GEOMETRY_SHADER_BIT,
	};

	bool OpenGL::Init(Window& window, uint32 antialiasing)
	{
		assert(!context); // already initialized

		// Store the thread ID that the OpenGL context runs on
		threadId = std::this_thread::get_id();

		m_window = &window;
		auto* sdlWnd = static_cast<SDL_Window*>(m_window->Handle());

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		// Create a context
		context = SDL_GL_CreateContext(sdlWnd);
		if(!context)
		{
            Logf("Failed to create OpenGL context: %s", Logger::Error, SDL_GetError());
            return false;
		}

		SDL_GL_MakeCurrent(sdlWnd, context);

		// macOS doesnt need glew
		#ifndef __APPLE__
		// To allow usage of experimental features
		glewExperimental = true;
		glewInit();
		#endif

		//#define LIST_OGL_EXTENSIONS
#ifdef LIST_OGL_EXTENSIONS
		Logf("Listing OpenGL Extensions:", Logger::Info);
		GLint n, i;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		for(i = 0; i < n; i++) {
			Logf("%s", Logger::Info, glGetStringi(GL_EXTENSIONS, i));
		}
#endif

#ifdef _DEBUG
		// Setup GL debug messages to go to the console
		if(glDebugMessageCallback && glDebugMessageControl)
		{
			Log("OpenGL Logging on.", Logger::Info);
			glDebugMessageCallback(GLDebugProc, 0);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, 0, GL_FALSE);
		}
#endif

		Logf("OpenGL Version: %s", Logger::Info, glGetString(GL_VERSION));
		Logf("OpenGL Shading Language Version: %s", Logger::Info, glGetString(GL_SHADING_LANGUAGE_VERSION));
		Logf("OpenGL Renderer: %s", Logger::Info, glGetString(GL_RENDERER));
		Logf("OpenGL Vendor: %s", Logger::Info, glGetString(GL_VENDOR));

		// Create pipeline for the program
		glGenProgramPipelines(1, &m_mainProgramPipeline);
		glBindProgramPipeline(m_mainProgramPipeline);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		return true;
	}

	Recti OpenGL::GetViewport() const
	{
		Recti vp;
		glGetIntegerv(GL_VIEWPORT, &vp.pos.x);
		return vp;
	}

	uint32 OpenGL::GetFramebufferHandle()
	{
		return GL_BACK;
	}

	void OpenGL::SetViewport(Recti vp)
	{
		glViewport(vp.pos.x, vp.pos.y, vp.size.x, vp.size.y);
	}

	void OpenGL::SetViewport(Vector2i size)
	{
		glViewport(0, 0, size.x, size.y);
	}

	bool OpenGL::IsOpenGLThread() const
	{
		return threadId == std::this_thread::get_id();
	}

	void OpenGL::SwapBuffers()
	{
		glFlush();
		SDL_GL_SwapWindow(static_cast<SDL_Window*>(m_window->Handle()));
	}

	OpenGL& OpenGL::instance()
	{
		static OpenGL instance;
		return instance;
	}

	void OpenGL::Unbind(ShaderType type)
	{
		if(m_activeShaders[(size_t)type] != nullptr)
		{
			glUseProgramStages(m_mainProgramPipeline, shaderStageMap[(size_t)type], 0);
			m_activeShaders[(size_t)type] = nullptr;
		}
	}

	void OpenGL::Bind(IShader* shader)
	{
		if(m_activeShaders[(size_t)shader->GetType()] != shader)
		{
			glUseProgramStages(m_mainProgramPipeline, shaderStageMap[(size_t)shader->GetType()], shader->Handle());
			m_activeShaders[(size_t)shader->GetType()] = shader;
		}
	}

	bool OpenGL::IsBound(IShader* shader)
	{
		return m_activeShaders[(size_t)shader->GetType()] == shader;
	}

#ifdef _WIN32
	void APIENTRY GLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	#else
	void GLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	#endif
	{
#define DEBUG_SEVERITY_HIGH                              0x9146
#define DEBUG_SEVERITY_MEDIUM                            0x9147
#define DEBUG_SEVERITY_LOW                               0x9148
#define DEBUG_SEVERITY_NOTIFICATION                      0x826B

		Logger::Severity mySeverity;
		switch(severity)
		{
		case DEBUG_SEVERITY_MEDIUM:
		case DEBUG_SEVERITY_HIGH:
			mySeverity = Logger::Warning;
			break;
		default:
			mySeverity = Logger::Info;
			break;
		}
		String msgString = String(message, message + length);
		Logf("GLDebug: %s", Logger::Info, msgString);
	}
}