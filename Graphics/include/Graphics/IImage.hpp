#pragma once

#include <Shared/String.hpp>
#include <Shared/Color.hpp>

namespace Graphics
{
	class IImage
	{
	public:
		virtual ~IImage() = default;

		virtual void SetSize(Vector2i size) = 0;
		virtual void ReSize(Vector2i size) = 0;
		virtual Vector2i GetSize() const = 0;
		virtual Colori* GetBits() = 0;
		virtual const Colori* GetBits() const = 0;
		virtual void SavePNG(const String& file) = 0;
	};
}