#pragma once

#include <Shared/Types.hpp>
#include <Shared/Rect.hpp>
#include "IImage.hpp"

namespace Graphics
{
	/*
		Sprite map
		Adding images to this will pack the image into a final image that contains all the added images
		After this the UV coordinates of these images can be asked for given and image index

		!! The packing is not optimal as the images are stacked for bottom to top and placed in columns based on their width
	*/
	class ISpriteMap
	{
	public:
		virtual ~ISpriteMap() = default;

		virtual uint32 AddSegment(const IImage& image) = 0;
		virtual void Clear() = 0;
		virtual IImage& GetImage() = 0;
		virtual unique_ptr<ITexture> GenerateTexture() = 0;
		virtual Recti GetCoords(uint32 nIndex) = 0;
	};
}