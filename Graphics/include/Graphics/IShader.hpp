#pragma once

#include <Shared/Types.hpp>
#include <Shared/String.hpp>
#include <Graphics/GL.hpp>

namespace Graphics
{
	// Enum of supported shader types
	enum class ShaderType
	{
		Vertex,
		Fragment,
		Geometry
	};



	// A single unlinked OpenGL shader
	class IShader
	{
	public:
		virtual ~IShader() = default;

		// Tries to hot-reload the shader program, only works if _DEBUG is defined
		// returns true if the program was changed and thus the handle value also changed
		virtual bool UpdateHotReload() = 0;

		virtual ShaderType GetType() const = 0;
		virtual bool IsBound() const = 0;
		virtual uint32 GetLocation(const String& name) const = 0;
		virtual uint32 Handle() = 0;
		virtual String GetOriginalName() const = 0;

		uint32 operator[](const char* name) const
		{
			return GetLocation(name);
		}
	};
}