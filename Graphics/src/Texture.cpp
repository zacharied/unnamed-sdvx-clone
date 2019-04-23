#include "stdafx.h"
#include "Texture.hpp"
#include "Image.hpp"

namespace Graphics
{
	float ITexture::CalculateHeight(float width)
	{
		Vector2 size = GetSize();
		float aspect = size.y / size.x;
		return aspect * width;
	}

	float ITexture::CalculateWidth(float height)
	{
		Vector2 size = GetSize();
		float aspect = size.x / size.y;
		return aspect * height;
	}

	auto Texture::Create() -> optional<unique_ptr<Texture>>
	{
		auto tex = make_unique<Texture>();
		if(!tex->Init())
			return {};
		return std::move(tex);
	}

	auto Texture::Create(const IImage& image) -> optional<unique_ptr<Texture>>
	{
		auto tex = make_unique<Texture>();
		if (!tex->Init())
			return {};
		tex->SetData(image.GetSize(), image.GetBits());
		return std::move(tex);
	}

	auto Texture::Create(const Vector2i& resolution) -> optional<unique_ptr<Texture>>
	{
		auto tex = make_unique<Texture>();
		if(!tex->Init())
			return {};

		tex->Init(resolution, TextureFormat::RGBA8);
		tex->SetWrap(TextureWrap::Clamp, TextureWrap::Clamp);
		tex->SetFromFrameBuffer(Vector2i{0, 0});
		return std::move(tex);
	}

	bool Texture::Init()
	{
		glGenTextures(1, &m_texture);
		return m_texture != 0;
	}

	void Texture::Init(Vector2i size, TextureFormat format)
	{
		m_format = format;
		m_size = size;
		uint32 ifmt;
		uint32 fmt;
		uint32 type;

		if(format == TextureFormat::D32)
		{
			ifmt = GL_DEPTH_COMPONENT32;
			fmt = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		}
		else if(format == TextureFormat::RGBA8)
		{
			ifmt = GL_RGBA8;
			fmt = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else
			assert(false);

		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, ifmt, size.x, size.y, 0, fmt, type, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		UpdateFilterState();
		UpdateWrap();
	}

	void Texture::SetData(Vector2i size, const void* pData)
	{
		m_format = TextureFormat::RGBA8;
		m_size = size;
		m_data = pData;
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
		glBindTexture(GL_TEXTURE_2D, 0);

		UpdateFilterState();
		UpdateWrap();
	}

	void Texture::SetFromFrameBuffer(Vector2i pos)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pos.x, pos.y, m_size.x, m_size.y);
		GLenum err;
		bool errored = false;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			Logf("OpenGL Error: 0x%p", Logger::Severity::Error, err);
			errored = true;
		}
		//assert(!errored);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::SetMipmaps(bool enabled)
	{
		if(enabled)
		{
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		m_mipmaps = enabled;
		UpdateFilterState();
	}

	void Texture::SetFilter(bool enabled, bool mipFiltering, float anisotropic)
	{
		m_mipFilter = mipFiltering;
		m_filter = enabled;
		m_anisotropic = anisotropic;
		assert(m_anisotropic >= 1.0f && m_anisotropic <= 16.0f);
		UpdateFilterState();
	}

	const Vector2i& Texture::GetSize() const
	{
		return m_size;
	}

	void Texture::Bind(uint32 index)
	{
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, m_texture);
	}

	uint32 Texture::Handle()
	{
		return m_texture;
	}

	void Texture::SetWrap(TextureWrap u, TextureWrap v)
	{
		m_wmode[0] = u;
		m_wmode[1] = v;
		UpdateWrap();
	}

	TextureFormat Texture::GetFormat() const
	{
		return m_format;
	}

	void Texture::UpdateWrap()
	{
		uint32 wmode[] = {
				GL_REPEAT,
				GL_MIRRORED_REPEAT,
				GL_CLAMP_TO_EDGE,
		};

		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wmode[(size_t)m_wmode[0]]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wmode[(size_t)m_wmode[1]]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::UpdateFilterState()
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);

		if(!m_mipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
		}
		else
		{
			if(m_mipFilter)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
			else
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
		}

		if(GL_TEXTURE_MAX_ANISOTROPY_EXT)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropic);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}