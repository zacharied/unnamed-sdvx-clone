#pragma once
#include <Graphics/IFont.hpp>
#include <Graphics/Text.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Graphics
{
	/*
		Font class, can create Text objects
	*/
	class Font : public IFont
	{
	public:
		~Font() override;

		static auto Create(const String& assetPath) -> optional<unique_ptr<Font>>;
		shared_ptr<IText> CreateText(const WString& str, uint32 nFontSize, TextOptions options = TextOptions::None);
	private:
		FT_Face m_face;
		Buffer m_data;

		Map<uint32, unique_ptr<FontSize>> m_sizes;
		uint32 m_currentSize = 0;

		Font() = default;
		bool Init(const String& assetPath);
		FontSize* GetSize(uint32 nSize);
	};
}
