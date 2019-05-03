#pragma once
#include <Shared/Thread.hpp>
#include <Graphics/GL.hpp>
#include <Graphics/Window.hpp>
#include "IShader.hpp"

namespace Graphics
{
#ifdef _WIN32
	void __stdcall GLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#else
	void GLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

	/*
		OpenGL context wrapper with common functionality
	*/
	using Shared::Recti;
	class OpenGL
	{
	public:
		OpenGL(const OpenGL&) = delete;
		OpenGL& operator=(const OpenGL&) = delete;
		~OpenGL();
		static OpenGL& instance();

		bool Init(Window& window, uint32 antialiasing);
		Recti GetViewport() const;
		uint32 GetFramebufferHandle();
		void SetViewport(Vector2i size);
		void SetViewport(Recti vp);

		void Bind(IShader* shader);
		void Unbind(ShaderType type);
		bool IsBound(IShader* shader);

		// Check if the calling thread is the thread that runs this OpenGL context
		bool IsOpenGLThread() const;

		virtual void SwapBuffers();

	private:
		array<IShader*, 3> m_activeShaders = { nullptr };
		uint32 m_mainProgramPipeline;
		Window* m_window;
		SDL_GLContext context;
		std::thread::id threadId;

		friend class IShader;
		friend class TextureRes;
		friend class MeshRes;
		friend class Shader_Impl;
		friend class RenderQueue;

		OpenGL() = default;
	};
}