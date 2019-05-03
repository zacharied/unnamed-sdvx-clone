#include "stdafx.h"
#include "Font.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"
#include <Shared/Timer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Graphics
{
	using Shared::Margin;
	using Shared::Recti;

	auto Font::Create(const String& assetPath) -> optional<unique_ptr<Font>>
	{
		struct EnableMaker : public Font { using Font::Font; };
		auto font = make_unique<EnableMaker>();
		if(!font->Init(assetPath))
			return {};

		return std::move(font);
	}

	Font::~Font()
	{
		m_sizes.clear();
		FT_Done_Face(m_face);
	}

	bool Font::Init(const String& assetPath)
	{
		File in;
		if(!in.OpenRead(assetPath))
			return false;

		m_data.resize(in.GetSize());
		if(m_data.empty())
			return false;

		in.Read(&m_data.front(), m_data.size());

		if(FT_New_Memory_Face(FontLibrary::instance().library, m_data.data(), (FT_Long)m_data.size(), 0, &m_face) != 0)
			return false;

		if(FT_Select_Charmap(m_face, FT_ENCODING_UNICODE) != 0)
			assert(false);

		return true;
	}

	FontSize* Font::GetSize(uint32 nSize)
	{
		if(m_currentSize != nSize)
		{
			FT_Set_Pixel_Sizes(m_face, 0, nSize);
			m_currentSize = nSize;
		}

		auto& fontLib = FontLibrary::instance();
		if(fontLib.fallbackFontSize != nSize)
		{
			FT_Set_Pixel_Sizes(fontLib.fallbackFont, 0, nSize);
			fontLib.fallbackFontSize = nSize;
		}

		auto it = m_sizes.find(nSize);
		if(it != m_sizes.end())
			return it->second.get();
		else
		{
			auto pMap = make_unique<FontSize>(m_face);
			auto obj = m_sizes.emplace(make_pair(nSize, std::move(pMap)));
			return (*obj.first).second.get();
		}
	}

	/*
	Ref<TextRes> CreateText(const WString& str, uint32 nFontSize, TextOptions options)
		{
			FontSize* size = GetSize(nFontSize);

			Text cachedText = size->cache.GetText(str);
			if(cachedText)
				return cachedText;

			struct TextVertex : public VertexFormat<Vector2, Vector2>
			{
				TextVertex(Vector2 point, Vector2 uv) : pos(point), tex(uv) {}
				Vector2 pos;
				Vector2 tex;
			};

			TextRes* ret = new TextRes();
			ret->mesh = MeshRes::Create(m_gl);

			float monospaceWidth = size->GetCharInfo(L'_').advance;

			Vector<TextVertex> vertices;
			Vector2 pen;
			for(wchar_t c : str)
			{
				const CharInfo& info = size->GetCharInfo(c);

				if(info.coords.size.x != 0 && info.coords.size.y != 0)
				{
					Vector2 corners[4];
					corners[0] = Vector2(0, 0);
					corners[1] = Vector2((float)info.coords.size.x, 0);
					corners[2] = Vector2((float)info.coords.size.x, (float)info.coords.size.y);
					corners[3] = Vector2(0, (float)info.coords.size.y);

					Vector2 offset = Vector2(pen.x, pen.y);
					offset.x += info.leftOffset;
					offset.y += nFontSize - info.topOffset;
					if((options & TextOptions::Monospace) != 0)
					{
						offset.x += (monospaceWidth - info.coords.size.x) * 0.5f;
					}
					pen.x = floorf(pen.x);
					pen.y = floorf(pen.y);

					vertices.emplace_back(offset + corners[2],
						corners[2] + info.coords.pos);
					vertices.emplace_back(offset + corners[0],
						corners[0] + info.coords.pos);
					vertices.emplace_back(offset + corners[1],
						corners[1] + info.coords.pos);

					vertices.emplace_back(offset + corners[3],
						corners[3] + info.coords.pos);
					vertices.emplace_back(offset + corners[0],
						corners[0] + info.coords.pos);
					vertices.emplace_back(offset + corners[2],
						corners[2] + info.coords.pos);
				}

				if(c == L'\n')
				{
					pen.x = 0.0f;
					pen.y += size->lineHeight;
					ret->size.y = pen.y;
				}
				else if(c == L'\t')
				{
					const CharInfo& space = size->GetCharInfo(L' ');
					pen.x += space.advance * 3.0f;
				}
				else
				{
					if((options & TextOptions::Monospace) != 0)
					{
						pen.x += monospaceWidth;
					}
					else
						pen.x += info.advance;
				}
				ret->size.x = std::max(ret->size.x, pen.x);
			}

			ret->size.y += size->lineHeight;

			ret->fontSize = size;
			ret->mesh->SetData(vertices);
			ret->mesh->SetPrimitiveType(PrimitiveType::TriangleList);

			Text textObj = Ref<TextRes>(ret);
			// Insert into cache
			size->cache.AddText(str, textObj);
			return textObj;
		}
	 */
}