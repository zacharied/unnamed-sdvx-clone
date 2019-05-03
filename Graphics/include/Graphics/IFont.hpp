#pragma once

#include <Graphics/IText.hpp>
#include <Shared/String.hpp>
#include <Shared/Types.hpp>

#ifdef None
#undef None
#endif

namespace Graphics
{
	/*
		Font class, can create Text objects
	*/
	class IFont
	{
	public:
		virtual ~IFont() = default;

		// Text rendering options
		enum class TextOptions
		{
			None = 0,
			Monospace = 0x1,
		};

	};
}