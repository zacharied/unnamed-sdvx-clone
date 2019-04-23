#pragma once
#include "IShader.hpp"

namespace Graphics
{
	class Shader : public IShader
	{
	public:
		~Shader() override;
		static auto Create(ShaderType type, const String& assetPath) -> optional<unique_ptr<Shader>>;

		// Tries to hot-reload the shader program, only works if _DEBUG is defined
		// returns true if the program was changed and thus the handle value also changed
		bool UpdateHotReload() override;
		ShaderType GetType() const override;
		bool IsBound() const override;
		uint32 GetLocation(const String& name) const override;
		uint32 Handle() override;
		String GetOriginalName() const override;

	private:
		ShaderType m_type;
		uint32 m_prog;

		String m_sourcePath;

#ifdef _WIN32 // Hot Reload detection on windows
		HANDLE m_changeNotification = INVALID_HANDLE_VALUE;
		uint64 m_lwt = -1;
#endif

		Shader() = default;
		bool Init(ShaderType type, const String& name);
		bool LoadProgram(uint32& programOut);
		void SetupChangeHandler(); // windows only
	};
}