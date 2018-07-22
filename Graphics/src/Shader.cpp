#include "stdafx.h"
#include "Shader.hpp"
#include <Graphics/ResourceManagers.hpp>
#include "OpenGL.hpp"

namespace Graphics
{
	uint32 typeMap[] =
	{
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_GEOMETRY_SHADER,
	};
	uint32 shaderStageMap[] =
	{
		GL_VERTEX_SHADER_BIT,
		GL_FRAGMENT_SHADER_BIT,
		GL_GEOMETRY_SHADER_BIT,
	};

	class Shader_Impl : public ShaderRes
	{
		ShaderType m_type;
		uint32 m_shader;
		OpenGL* m_gl;

		String m_sourcePath;

		// Hot Reload detection on windows
#ifdef _WIN32
		HANDLE m_changeNotification = INVALID_HANDLE_VALUE;
		uint64 m_lwt = -1;
#endif
	public:
		Shader_Impl(OpenGL* gl) : m_gl(gl)
		{
		}
		~Shader_Impl()
		{
			// Cleanup OpenGL resource
			if(glIsShader(m_shader))
			{
				glDeleteShader(m_shader);
			}

#ifdef _WIN32
			// Close change notification handle
			if(m_changeNotification != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_changeNotification);
			}
#endif
		}
		void SetupChangeHandler()
		{
#ifdef _WIN32
			if(m_changeNotification != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_changeNotification);
				m_changeNotification = INVALID_HANDLE_VALUE;
			}

			String rootFolder = Path::RemoveLast(m_sourcePath);
			m_changeNotification = FindFirstChangeNotificationA(*rootFolder, false, FILE_NOTIFY_CHANGE_LAST_WRITE);
#endif
		}
		bool LoadProgram()
		{
			File in;
			if(!in.OpenRead(m_sourcePath))
				return false;

			String sourceStr;
			sourceStr.resize(in.GetSize());
			if(sourceStr.size() == 0)
				return false;

			in.Read(&sourceStr.front(), sourceStr.size());
			const GLint programsize = sourceStr.size();

			const char* pChars = *sourceStr;
			glShaderSource(m_shader, 1, &pChars, &programsize);
			glCompileShader(m_shader);

			//programOut = glCreateShaderProgramv(typeMap[(size_t)m_type], 1, &pChars);
			//if(programOut == 0)
			//	return false;

			int nStatus = 0;
			glGetShaderiv(m_shader, GL_COMPILE_STATUS, &nStatus);
			if(nStatus == GL_FALSE)
			{
				static char infoLogBuffer[2048];
				int s = 0;
				glGetShaderInfoLog(m_shader, sizeof(infoLogBuffer), &s, infoLogBuffer);

				Logf("Shader program compile log for %s: %s", Logger::Error, m_sourcePath, infoLogBuffer);
				return false;
			}

			// Shader hot-reload in debug mode
#if defined(_DEBUG) && defined(_WIN32)
			// Store last write time
			m_lwt = in.GetLastWriteTime();
			SetupChangeHandler();
#endif
			return true;
		}
		bool UpdateHotReload()
		{
#ifdef _WIN32
			if(m_changeNotification != INVALID_HANDLE_VALUE)
			{
				if(WaitForSingleObject(m_changeNotification, 0) == WAIT_OBJECT_0)
				{
					uint64 newLwt = File::GetLastWriteTime(m_sourcePath);
					if(newLwt != -1 && newLwt > m_lwt)
					{
						if(LoadProgram())
						{
							return true;
						}
					}

					// Watch for new change
					SetupChangeHandler();
				}
			}
#endif
			return false;
		}

		bool Init(ShaderType type, const String& name)
		{
			m_sourcePath = Path::Normalize(name);
			m_type = type;
			m_shader = glCreateShader(typeMap[(size_t)m_type]);
			return LoadProgram();
		}

		virtual uint32 Handle() override
		{
			return m_shader;
		}

		String GetOriginalName() const
		{
			return m_sourcePath;
		}
	};

	Shader ShaderRes::Create(class OpenGL* gl, ShaderType type, const String& assetPath)
	{
		Shader_Impl* pImpl = new Shader_Impl(gl);
		if(!pImpl->Init(type, assetPath))
		{
			delete pImpl;
			return Shader();
		}
		else
		{
			return GetResourceManager<ResourceType::Shader>().Register(pImpl);
		}
	}
	void ShaderRes::Unbind(class OpenGL* gl, ShaderType type)
	{
		if(gl->m_activeShaders[(size_t)type] != 0)
		{
			glUseProgramStages(gl->m_mainProgramPipeline, shaderStageMap[(size_t)type], 0);
			gl->m_activeShaders[(size_t)type] = 0;
		}
	}
}
