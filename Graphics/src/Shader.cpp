#include "stdafx.h"
#include "Shader.hpp"
#include "OpenGL.hpp"

namespace Graphics
{
	uint32 typeMap[] =
	{
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_GEOMETRY_SHADER,
	};

	Shader::~Shader()
	{
		// Cleanup OpenGL resource
		if(glIsProgram(m_prog))
			glDeleteProgram(m_prog);

#ifdef _WIN32
		// Close change notification handle
		if(m_changeNotification != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_changeNotification);
		}
#endif
	}

	auto Shader::Create(ShaderType type, const String& assetPath) -> optional<unique_ptr<Shader>>
	{
		auto shader = make_unique<Shader>();
		if (!shader->Init(type, assetPath))
			return {};
		return std::move(shader);
	}

	bool Shader::Init(ShaderType type, const String& name)
	{
		m_sourcePath = Path::Normalize(name);
		m_type = type;
		return LoadProgram(m_prog);
	}

	bool Shader::LoadProgram(uint32& programOut)
	{
		File in;
		if(!in.OpenRead(m_sourcePath))
			return false;

		String sourceStr;
		sourceStr.resize(in.GetSize());
		if(sourceStr.empty())
			return false;

		in.Read(&sourceStr.front(), sourceStr.size());

		const char* pChars = *sourceStr;
		programOut = glCreateShaderProgramv(typeMap[(size_t)m_type], 1, &pChars);
		if(programOut == 0)
			return false;

		int nStatus = 0;
		glGetProgramiv(programOut, GL_LINK_STATUS, &nStatus);
		if(nStatus == 0)
		{
			static char infoLogBuffer[2048];
			int s = 0;
			glGetProgramInfoLog(programOut, sizeof(infoLogBuffer), &s, infoLogBuffer);

			Logf("Shader program compile log for %s: %s", Logger::Error, m_sourcePath, infoLogBuffer);
			return false;
		}

#if defined(_DEBUG) && defined(_WIN32) // Shader hot-reload in debug mode
		// Store last write time
		m_lwt = in.GetLastWriteTime();
		SetupChangeHandler();
#endif
		return true;
	}

	void Shader::SetupChangeHandler()
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

	bool Shader::UpdateHotReload()
	{
#ifdef _WIN32
		if(m_changeNotification != INVALID_HANDLE_VALUE)
			{
				if(WaitForSingleObject(m_changeNotification, 0) == WAIT_OBJECT_0)
				{
					uint64 newLwt = File::GetLastWriteTime(m_sourcePath);
					if(newLwt != -1 && newLwt > m_lwt)
					{
						uint32 newProgram = 0;
						if(LoadProgram(newProgram))
						{
							// Successfully reloaded
							m_prog = newProgram;
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

	ShaderType Shader::GetType() const
	{
		return m_type;
	}

	bool Shader::IsBound() const
	{
		return false;
	}

	uint32 Shader::GetLocation(const String& name) const
	{
		return glGetUniformLocation(m_prog, name.c_str());
	}

	uint32 Shader::Handle()
	{
		return m_prog;
	}

	String Shader::GetOriginalName() const
	{
		return m_sourcePath;
	}
}
