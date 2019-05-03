#pragma once

#include <Shared/VectorMath.hpp>
#include <Shared/Log.hpp>
#include <Graphics/Text.hpp>
#include <Graphics/SpriteMap.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Image.hpp>
#include <Shared/File.hpp>

namespace Graphics
{
	ITexture* Text::GetTexture()
	{
		return fontSize->GetTextureMap();
	}

	void Text::Draw()
	{
		GetTexture()->Bind(0);
		mesh->Draw();
	}

	IMesh* Text::GetMesh()
	{
		return mesh.get();
	}

	Vector2 Text::GetSize()
	{
		return size;
	}

	FontSize::FontSize(FT_Face& face) : face(face)
	{
		auto res = SpriteMap::Create();
		assert(res); // TODO: (factory)
		auto tex = Texture::Create();
		assert(tex); // TODO: (factory)
		textureMap = std::move(*tex);
		lineHeight = (float) face->size->metrics.height / 64.0f;
	}

	const CharInfo& FontSize::GetCharInfo(wchar_t t)
	{
		auto it = infoByChar.find(t);
		if (it == infoByChar.end())
			return AddCharInfo(t);
		return infos[it->second];
	}

	ITexture* FontSize::GetTextureMap()
	{
		if (bUpdated)
		{
			textureMap = spriteMap->GenerateTexture();
			bUpdated = false;
		}
		return textureMap.get();
	}

	const CharInfo& FontSize::AddCharInfo(wchar_t t)
	{
		bUpdated = true;
		infoByChar.Add(t, (uint32) infos.size());
		infos.emplace_back();
		CharInfo& ci = infos.back();

		FT_Face* pFace = &face;

		ci.glyphID = FT_Get_Char_Index(*pFace, t);
		if (ci.glyphID == 0)
		{
			pFace = &FontLibrary::instance().fallbackFont;
			ci.glyphID = FT_Get_Char_Index(*pFace, t);
		}
		FT_Load_Glyph(*pFace, ci.glyphID, FT_LOAD_DEFAULT);

		if ((*pFace)->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Render_Glyph((*pFace)->glyph, FT_RENDER_MODE_NORMAL);
		}

		ci.topOffset = (*pFace)->glyph->bitmap_top;
		ci.leftOffset = (*pFace)->glyph->bitmap_left;
		ci.advance = (float) (*pFace)->glyph->advance.x / 64.0f;

		auto img = Image::Create(Vector2i{static_cast<int>((*pFace)->glyph->bitmap.width), static_cast<int>((*pFace)->glyph->bitmap.rows)});
		assert(img); // TODO: (factory);
		Colori* pDst = (*img)->GetBits();
		uint8* pSrc = (*pFace)->glyph->bitmap.buffer;
		uint32 nLen = (*pFace)->glyph->bitmap.width * (*pFace)->glyph->bitmap.rows;
		for (uint32 i = 0; i < nLen; i++)
		{
			pDst[0].w = pSrc[0];
			reinterpret_cast<VectorMath::VectorBase<uint8, 3>&>(pDst[0]) = VectorMath::VectorBase<uint8, 3>(255, 255,
																											255);
			pSrc++;
			pDst++;
		}
		uint32 nIndex = spriteMap->AddSegment(**img);
		ci.coords = spriteMap->GetCoords(nIndex);

		return ci;
	}

	void TextCache::Update()
	{
		float currentTime = timer.SecondsAsFloat();
		for (auto it = begin(); it != end();)
		{
			float durationSinceUsed = currentTime - it->second.lastUsage;
			if (durationSinceUsed > 1.0f)
			{
				it = erase(it);
				continue;
			}
			it++;
		}
	}

	optional<Text*> TextCache::GetText(const WString& key)
	{
		auto it = find(key);
		if (it != end())
		{
			it->second.lastUsage = timer.SecondsAsFloat();
			return it->second.text.get();
		}
		return {};
	}

	void TextCache::AddText(const WString& key, unique_ptr<Text> obj)
	{
		Update();
		auto it = find(key);
		if(it == end())
			insert(std::make_pair(key, CachedText{std::move(obj), timer.SecondsAsFloat()}));
	}

	FontLibrary& FontLibrary::instance()
	{
		static FontLibrary instance;
		return instance;
	}

	FontLibrary::FontLibrary()
	{
		assert(FT_Init_FreeType(&library) == FT_Err_Ok);
		if(!LoadFallbackFont())
			Log("Failed to load embeded fallback font", Logger::Warning);
	}

	FontLibrary::~FontLibrary()
	{
		FT_Done_Face(fallbackFont);
		FT_Done_FreeType(library);
	}

	bool FontLibrary::LoadFallbackFont()
	{
		bool success = true;

		// Load fallback font
		File file;
		if(!file.OpenRead("fonts/fallbackfont.otf"))
			return false;

		loadedFallbackFont.resize(file.GetSize());
		file.Read(loadedFallbackFont.data(), loadedFallbackFont.size());
		file.Close();

		if(FT_New_Memory_Face(library, loadedFallbackFont.data(), (uint32)loadedFallbackFont.size(), 0, &fallbackFont) != 0)
			return false;

		if(FT_Select_Charmap(fallbackFont, FT_ENCODING_UNICODE) != 0)
			return false;

		return true;
	}
}
