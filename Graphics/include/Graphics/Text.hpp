#pragma once

#include <Shared/Types.hpp>
#include <Shared/Map.hpp>
#include <Shared/Timer.hpp>
#include <Graphics/IText.hpp>
#include <Graphics/ITexture.hpp>
#include <Graphics/ISpriteMap.hpp>
#include <Shared/Buffer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Graphics
{
	class Text : public IText
	{
	public:
		~Text() override = default;

		ITexture* GetTexture() override;
		IMesh* GetMesh() override;
		void Draw() override;
		Vector2 GetSize() override;

	private:
		friend class Font;

		Vector2 size;
		unique_ptr<IMesh> mesh;
		struct FontSize* fontSize;
	};

	struct CachedText
	{
		shared_ptr<IText> text;
		float lastUsage;
	};

	struct CharInfo
	{
		uint32 glyphID;
		float advance;
		int32 leftOffset;
		int32 topOffset;
		Recti coords;
	};

	// Prevents continuous recreation of text that doesn't change
	class TextCache : public Map<WString, CachedText>
	{
	public:
		void Update();
		optional<shared_ptr<IText>> GetText(const WString& key);
		void AddText(const WString& key, shared_ptr<IText> obj);

	private:
		Timer timer;
	};

	struct FontSize
	{
		unique_ptr<ISpriteMap> spriteMap;
		unique_ptr<ITexture> textureMap;
		FT_Face face;
		Vector<CharInfo> infos;
		Map<wchar_t, uint32> infoByChar;
		bool bUpdated = false;
		float lineHeight;
		TextCache cache;

		explicit FontSize(FT_Face& face);
		~FontSize() = default;

		const CharInfo& GetCharInfo(wchar_t t);
		ITexture* GetTextureMap();

	private:
		const CharInfo& AddCharInfo(wchar_t t);
	};

	struct FontLibrary
	{
		static FontLibrary& instance();

		FontLibrary();
		~FontLibrary();
		bool LoadFallbackFont();
		Buffer loadedFallbackFont;

		FT_Library library;
		FT_Face fallbackFont;
		uint32 fallbackFontSize = 0;
	};
}