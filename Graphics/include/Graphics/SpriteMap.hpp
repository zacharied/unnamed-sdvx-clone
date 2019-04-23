#pragma once

#include "ISpriteMap.hpp"

namespace Graphics
{
	struct Category
	{
		uint32 width = 0;
		Vector2i offset;
		Vector<uint32> segments;
	};

	struct Segment
	{
		Recti coords;
	};

	class SpriteMap : public ISpriteMap
	{
	public:
		~SpriteMap() override;

		uint32 AddSegment(const IImage& image) override;
		void Clear() override;
		IImage& GetImage() override;
		unique_ptr<ITexture> GenerateTexture() override;
		Recti GetCoords(uint32 nIndex) override;

	private:
		// The image that contains the current data
		unique_ptr<IImage> m_image;

		// Used size over the X axis
		int32 m_usedSize = 0;

		// Linear index of al segements
		Vector<Segment> m_segments;
		Vector<Category> m_widths;
		std::multimap<uint32, uint32> m_categoryByWidth;

		SpriteMap();

		// Returns the category that has space for the requested size
		Category& AssignCategory(Vector2i requestedSize);
		static void CopySubImage(IImage& dst, const IImage& src, Vector2i dstPos);
	};
}